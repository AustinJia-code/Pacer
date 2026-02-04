/**
 * @file sender.cpp
 * @brief Sends packets
 */

#include "network.h"
#include "helpers.h"
#include "consts.h"
#include <cstdlib>
#include <iostream>
#include <unistd.h>

/**
 * Runner
 */
int main (int argc, char* argv[])
{
    // Read ports
    if (argc < 2)
    {
        std::cerr << "Usage: ./sender [port]" << std::endl;
        return EXIT_FAILURE;
    }

    int port = atoi (argv[1]);

    int sock = create_udp_socket ();
    if (sock < 0)
        return EXIT_FAILURE;

    sockaddr_in dest = make_dest_addr ("127.0.0.1", port);

    // Send
    if (debug)
        std::cout << "Sending..." << std::endl;

    Packet packet;

    for (id_t id = 0; id < (2 << 10); ++id)
    {
        packet.id = id;
        packet.byte_count = 0;

        if (debug)
            std::cout << "Sending ID " << packet.id << std::endl;

        if (send_packet (sock, packet, dest) < 0)
            std::cerr << "Issue sending to socket" << std::endl;

        usleep (sec_to_usec ({sec_t {0.1}}));
    }
}
