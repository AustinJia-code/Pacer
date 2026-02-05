/**
 * @file receiver.cpp
 * @brief Receives packets
 */

#include "network.h"
#include "consts.h"
#include <cstdlib>
#include <iostream>

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

    DataPacket data_packet;

    while (true)
    {
        // Receive data
        if (receive_data (sock, data_packet) < 1)
        {
            std::cerr << "Issue reading data from socket" << std::endl;
            continue;
        }

        // Send ack
        AckPacket ack = {.header = {.type = PacketType::Ack,
                                    .id = data_packet.header.id}};

        if (send_ack (sock, ack, ack_dest_addr) < 0)
            std::cerr << "Issue sending ack to socket" << std::endl;

        if (debug)
            std::cout << "ID " << data_packet.header.id << " Received."
                      << std::endl;
    }
}
