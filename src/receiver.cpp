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
    // Read ports
    if (argc < 2)
    {
        std::cerr << "Usage: ./receiver [port]" << std::endl;
        return EXIT_FAILURE;
    }

    int port = atoi (argv[1]);

    int sock = create_udp_socket ();
    if (sock < 0)
        return EXIT_FAILURE;

    bind_socket (sock, port);

    // Listen
    if (debug)
        std::cout << "Receiving..." << std::endl;

    Packet packet;

    while (true)
    {
        if (receive_packet (sock, packet) < 1)
        {
            std::cerr << "Issue reading from socket" << std::endl;
            continue;
        }

        if (debug)
            std::cout << "ID " << packet.id << " Received." << std::endl;
    }
}
