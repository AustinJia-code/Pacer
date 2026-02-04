/**
 * @file sender.cpp
 * @brief Sends packets
 */

#include "helpers.h"
#include "packet.h"
#include "consts.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <iostream>

/**
 * Runner
 */
int main ()
{
    int sock;
    if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        return EXIT_FAILURE;

    // Build addr
    sockaddr_in dest {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons (port);
    inet_pton (AF_INET, "127.0.0.1", &dest.sin_addr);

    // Send
    if (debug)
        std::cout << "SENDING..." << std::endl;

    Packet packet;
    for (id_t id = 0; id < (2 << 10); ++id)
    {
        packet.id = htonl (id);
        packet.byte_count = 0;

        sendto (sock, &packet, sizeof (packet.id) + sizeof (packet.byte_count) +
                               packet.byte_count,
                0, (sockaddr*) &dest, sizeof (dest));

        if (debug)
            std::cout << "Sending ID " << id << std::endl;

        usleep (sec_to_usec ({sec_t {0.1}}));
    }
}