/**
 * @file emulator.cpp
 * @brief Emulate network hazards between sender and receiver.
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
    if (argc < 3)
    {
        std::cerr << "Usage: ./emulator [receiver port] [sender port]"
                  << std::endl;
        return EXIT_FAILURE;
    }

    // Sockets
    int receive_port = atoi (argv[1]);
    int receive_sock = create_udp_socket ();
    if (receive_sock < 0)
        return EXIT_FAILURE;
    bind_socket (receive_sock, receive_port);

    int send_port = atoi (argv[2]);
    int send_sock = create_udp_socket ();
    if (send_sock < 0)
        return EXIT_FAILURE;
    sockaddr_in dest = make_dest_addr ("127.0.0.1", send_port);

    // Hazards + pass-through
    if (debug)
        std::cout << "Emulating..." << std::endl;

    Packet packet;

    while (true)
    {
        if (receive_packet (receive_sock, packet) < 1)
        {
            std::cerr << "Issue reading from socket" << std::endl;
            continue;
        }

        if (debug)
            std::cout << "Applying hazards" << std::endl;

        // TODO: APPLY HAZARDS!!! RAH

        if (debug)
            std::cout << "Forwarding ID " << packet.id << std::endl;

        send_packet (send_sock, packet, dest);
    }
}
