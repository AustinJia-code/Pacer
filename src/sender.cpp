/**
 * @file sender.cpp
 * @brief Sends packets
 */

#include "network.h"
#include "helpers.h"
#include "consts.h"
#include "packet.h"
#include "metrics.h"
#include "display.h"
#include "pacer.h"
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <set>
#include <string>
#include <optional>
#include <cstring>

static constexpr size_t PAYLOAD_SIZE = 1024;

struct WindowSlot
{
    DataPacket packet;
    bool ack;
    size_t transmissions;
};

/**
 * Handler for packet window
 */
class Window
{
private:
    static constexpr size_t max_size = 10;

public:
    // <DataPacket, ack received>
    std::array<WindowSlot, max_size> out_buffer = {};

    // number of slots populated
    size_t n = 0;

    /**
     * Move start of the internal buffer to out_buffer and update n
     * Returns number of slots opened
     */
    size_t try_shift ()
    {
        size_t ind = 0;
        while (ind < n && out_buffer[ind].ack)
            ++ind;

        std::move
        (
            out_buffer.begin () + ind,
            out_buffer.end (),
            out_buffer.begin ()
        );

        n -= ind;
        return ind;
    }

    /**
     * Returns false if add unsuccessful
     */
    bool add (DataPacket packet)
    {
        if (n == max_size)
            return false;

        out_buffer[n++] = WindowSlot {.packet = packet,
                                      .ack = false,
                                      .transmissions = 0};
        return true;
    }

    /**
     * Set packets as acknowledged
     */
    void set_acks (const std::set<id_t> ack_ids)
    {
        for (size_t ind = 0; ind < n; ++ind)
            if (ack_ids.contains (out_buffer[ind].packet.header.id))
                out_buffer[ind].ack = true;
    }

    /**
     * Count of unacked packets in window
     */
    size_t unacked () const
    {
        size_t count = 0;
        for (size_t ind = 0; ind < n; ++ind)
            if (!out_buffer[ind].ack)
                ++count;
        return count;
    }
};

/**
 * Runner
 */
int main (int argc, char* argv[])
{
    /**** START RECEIVE ****/
    if (argc < 3)
    {
        std::cerr << "Usage: ./sender [bind port] [dest port] [--paced]"
                  << std::endl;
        return EXIT_FAILURE;
    }

    int bind_port = atoi (argv[1]);
    int dest_port = atoi (argv[2]);

    // --paced: token bucket rate shaping
    bool paced = false;
    for (int i = 3; i < argc; ++i)
        if (std::string (argv[i]) == "--paced")
            paced = true;

    // 75 pkt/s avg, burst cap 5
    std::optional<TokenBucket> pacer;
    if (paced)
        pacer.emplace (75.0, 5);

    int sock = create_udp_socket ();
    if (sock < 0)
        return EXIT_FAILURE;

    // Ack receive
    if (bind_socket (sock, bind_port) < 0)
    {
        std::cerr << "Issue binding sender" << std::endl;
        return EXIT_FAILURE;
    }

    // Data send
    sockaddr_in data_dest_addr = make_dest_addr ("127.0.0.1", dest_port);

    /**** START SEND ****/
    DataPacket data_packet;
    AckPacket ack_packet;

    Window window {};
    bool complete = false;
    id_t last_id = -1;      // intentional underflow

    SenderMetrics metrics {};
    Display display {};

    // Rate tracking (1-second rolling window)
    ms_t rate_window_start = get_time_ms ();
    size_t rate_window_count = 0;
    float current_rate = 0.0f;
    size_t last_burst = 0;

    while (!complete)
    {
        // receive all acks, set in window
        std::set<id_t> ack_ids = receive_all_acks (sock, ack_timeout);
        window.set_acks (ack_ids);

        for (id_t ack_id : ack_ids)
            display.add_event ("Acked      ID " + std::to_string (ack_id));

        // shift window
        window.try_shift ();

        // fill window, then send — so burst = full window
        for (id_t id = last_id + 1; id < (2 << 10); ++id)
        {
            data_packet.header = {.type = PacketType::Data, .id = id};
            data_packet.byte_count = PAYLOAD_SIZE;
            memset (data_packet.payload.data (), (byte_t) (id & 0xFF),
                    PAYLOAD_SIZE);

            if (!window.add (data_packet))
                break;

            last_id = id;
        }

        // send all unacked in window
        size_t burst = 0;
        for (size_t ind = 0; ind < window.n; ++ind)
        {
            if (window.out_buffer[ind].ack)
                continue;

            // Rate shaping: skip if bucket empty
            if (pacer && !pacer->try_consume ())
                break;

            id_t id = window.out_buffer[ind].packet.header.id;
            bool is_retransmit = window.out_buffer[ind].transmissions > 0;

            if (send_data (sock, window.out_buffer[ind].packet,
                           data_dest_addr) < 0)
            {
                display.add_event ("Send fail  ID " + std::to_string (id));
                continue;
            }

            if (is_retransmit)
                display.add_event ("Retransmit ID " + std::to_string (id));
            else
                display.add_event ("Transmit   ID " + std::to_string (id));

            if (!is_retransmit)
                ++metrics.unique_sent;

            ++window.out_buffer[ind].transmissions;
            ++metrics.total_sent;
            metrics.bytes_sent += window.out_buffer[ind].packet.byte_count;
            ++burst;
            ++rate_window_count;
        }
        if (burst > 0)
            last_burst = burst;

        // Update rolling rate (1-second window)
        ms_t now = get_time_ms ();
        ms_t elapsed = now - rate_window_start;
        if (elapsed >= 1000)
        {
            current_rate = rate_window_count / (elapsed / 1000.0f);
            rate_window_count = 0;
            rate_window_start = now;
        }

        // Render display
        char rate_buf[32];
        std::snprintf (rate_buf, sizeof (rate_buf), "%.0f", current_rate);

        char eff_buf[32];
        float efficiency = metrics.total_sent > 0
            ? (100.0f * metrics.unique_sent / metrics.total_sent) : 100.0f;
        std::snprintf (eff_buf, sizeof (eff_buf), "%.0f%%", efficiency);

        std::string mode = paced ? "Paced" : "Unshaped";
        std::string stats =
            "  [" + mode + "]" +
            "  Burst: " + std::to_string (last_burst) +
            "  |  Rate: " + rate_buf + " pkt/s" +
            "  |  Efficiency: " + eff_buf +
            "  |  Sent: " + std::to_string (metrics.unique_sent) +
                "/" + std::to_string (metrics.total_sent) +
            "  |  In Flight: " + std::to_string (window.unacked ()) + "/10";

        display.render ("--- Sender ---", stats);

        usleep (sec_to_us ({sec_t {0.1}}));
    }
}
