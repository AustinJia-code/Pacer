/**
 * @file receiver.cpp
 * @brief Receives packets
 */

#include "packet.h"
#include <arpa/inet.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>
#include <stdlib.h>
#include <iostream>
#include "consts.h"

/**
 * Runner
 */
int main ()
{
    // IPv4, UDP
    int sock;
    if ((sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
        return EXIT_FAILURE;
    
    // Build IPv4
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons (port);

    bind (sock, (sockaddr*) &addr, sizeof (addr));

    // Listen
    if (debug)
        std::cout << "RECEIVING..." << std::endl;

    Packet packet;
    while (true)
    {
        ssize_t byte_count;
        if (byte_count = recv (sock, &packet, sizeof (packet), 0) < 1)
            continue;
        
        id_t id = ntohl (packet.id);
        if (debug)
            std::cout << "ID " << id << " Received." << std::endl;
    }
}