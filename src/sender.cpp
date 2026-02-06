/**
 * @file sender.cpp
 * @brief Sends packets
 */

#include "network.h"
#include "helpers.h"
#include "consts.h"
#include "packet.h"
#include <cstdlib>
#include <iostream>
#include <unistd.h>
#include <set>

/**
 * Handler for packet window
 */
class Window
{
private:
    static constexpr size_t max_size = 10;

public:
    // <DataPacket, ack received>
    std::array<std::pair<DataPacket, bool>, max_size> out_buffer = {};

    // number of slots populated
    size_t n = 0;   

    /**
     * Move start of the internal buffer to out_buffer and update n
     * Returns number of slots opened
     */
    size_t try_shift ()
    {
        size_t ind = 0;
        while (ind < n && out_buffer[ind].second)
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
        
        out_buffer[n++] = std::make_pair (packet, false);        
        return true;
    }

    /**
     * Set packets as acknowledged
     */
    void set_acks (const std::set<id_t> ack_ids)
    {
        for (size_t ind = 0; ind < n; ++ind)
            if (ack_ids.contains (out_buffer[ind].first.header.id))
                out_buffer[ind].second = true;
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
        std::cerr << "Usage: ./sender [bind port] [dest port]" << std::endl;
        return EXIT_FAILURE;
    }

    int bind_port = atoi (argv[1]);
    int dest_port = atoi (argv[2]);

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
    if (debug)
        std::cout << "Sending..." << std::endl;

    DataPacket data_packet;
    AckPacket ack_packet;
    
    Window window {};
    bool complete = false;
    id_t last_id = 0;

    while (!complete)
    {
        // receive all acks, set in window
        std::set<id_t> ack_ids = receive_all_acks (sock, ack_timeout);
        window.set_acks (ack_ids);
        
        // shift window
        window.try_shift ();
        
        // for all in window, if no ack resend
        for (size_t ind = 0; ind < window.n; ++ind)
            if (send_data (sock, window.out_buffer[ind].first,
                           data_dest_addr) < 0)
                std::cerr << "Issue sending to socket" << std::endl;

        // add to end until window_size reached
        for (id_t id = last_id; last_id < (2 << 10); ++id)
        {
            data_packet.header = {.type = PacketType::Data, .id = id};
            data_packet.byte_count = 0;

            if (!window.add (data_packet))
                break;
            
            last_id = id;
        }

        usleep (sec_to_us ({sec_t {0.1}}));
    }
}
