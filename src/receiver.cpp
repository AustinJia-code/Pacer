/**
 * @file receiver.cpp
 * @brief Receives packets
 */

#include "network.h"
#include "consts.h"
#include <cstdlib>
#include <iostream>
#include <set>

/**
 * Handle incoming data packet, return true on success
 */
bool handle_data_packet (const DataPacket& data_packet)
{
    std::cout << "ID " << data_packet.header.id << " Received."
                << std::endl;
    return true;
}

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
    if (debug)
        std::cout << "Receiving..." << std::endl;

    std::set<DataPacket> in_buf {};
    DataPacket data_packet {};
    id_t last_handled_id = -1;      // intentional overflow

    while (true)
    {
        // Receive data
        if (receive_data (sock, data_packet) < 1)
        {
            std::cerr << "Issue reading data from socket" << std::endl;
            continue;
        }

        if ((data_packet.header.id > last_handled_id) || (last_handled_id == -1))
            in_buf.insert (data_packet);

        // Send ack
        AckPacket ack = {.header = {.type = PacketType::Ack,
                                    .id = data_packet.header.id}};

        if (send_ack (sock, ack, ack_dest_addr) < 0)
            std::cerr << "Issue sending ack to socket" << std::endl;

        while (!in_buf.empty ())
        {
            auto handle_it = in_buf.begin ();
            // If cannot handle contiguous, keep waiting
            if (handle_it->header.id != (last_handled_id + 1))
                break;
                
            handle_data_packet (*handle_it);
            ++last_handled_id;

            in_buf.erase (handle_it);
        }
    }
}
