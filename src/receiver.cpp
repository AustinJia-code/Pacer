/**
 * @file receiver.cpp
 * @brief Receives packets
 */

#include "network.h"
#include "metrics.h"
#include <cstdlib>
#include <iostream>
#include <set>
#include <string>

/**
 * Runner
 */
int main (int argc, char* argv[])
{
    /**** PARSE ARGS ****/
    if (argc < 3)
    {
        std::cerr << "Usage: ./receiver [bind port] [ack dest port]" << std::endl;
        return EXIT_FAILURE;
    }

    int bind_port = atoi (argv[1]);
    int ack_dest_port = atoi (argv[2]);

    int sock = create_udp_socket ();
    if (sock < 0)
        return EXIT_FAILURE;

    // Data receive
    if (bind_socket (sock, bind_port) < 0)
    {
        std::cerr << "Issue binding receiver" << std::endl;
        return EXIT_FAILURE;
    }

    // Ack send
    sockaddr_in ack_dest_addr = make_dest_addr ("127.0.0.1", ack_dest_port);

    /**** START RECEIVE ****/
    std::set<DataPacket> in_buf {};
    DataPacket data_packet {};
    id_t last_handled_id = -1;      // intentional underflow

    ReceiverMetrics metrics {};
    Display display {};

    while (true)
    {
        // Receive data
        if (receive_data (sock, data_packet) < 1)
            continue;

        ++metrics.total_received;
        id_t id = data_packet.header.id;
        bool is_new = false;

        if ((id > last_handled_id) || (last_handled_id == (id_t)-1))
            is_new = in_buf.insert (data_packet).second;

        if (is_new)
        {
            ++metrics.unique_received;
            display.add_event ("Received  ID " + std::to_string (id));
        }
        else
        {
            display.add_event ("Duplicate ID " + std::to_string (id));
        }

        // Send ack
        AckPacket ack = {.header = {.type = PacketType::Ack,
                                    .id = data_packet.header.id}};

        if (send_ack (sock, ack, ack_dest_addr) < 0)
            display.add_event ("Ack fail  ID " + std::to_string (id));

        // Deliver contiguous packets
        while (!in_buf.empty ())
        {
            auto it = in_buf.begin ();
            if (it->header.id != (last_handled_id + 1))
                break;

            display.add_event ("Delivered ID " + std::to_string (it->header.id));
            ++last_handled_id;

            in_buf.erase (it);
        }

        // Render display
        std::string stats =
            "  Received: " + std::to_string (metrics.total_received) +
            "  |  Unique: " + std::to_string (metrics.unique_received) +
            "  |  Duplicates: " +
                std::to_string (metrics.total_received - metrics.unique_received) +
            "  |  Buffered: " + std::to_string (in_buf.size ());

        display.render ("--- Receiver ---", stats);
    }
}
