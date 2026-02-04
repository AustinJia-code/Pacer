/**
 * @file network.h
 * @brief Shared UDP socket helpers
 */

#pragma once

#include "packet.h"
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <unistd.h>

/**
 * Create a UDP socket. Returns fd or -1 on failure.
 */
inline int create_udp_socket ()
{
    int sock = socket (AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (sock < 0)
        std::cerr << "Could not acquire socket" << std::endl;

    return sock;
}

/**
 * Build a sockaddr_in bound to INADDR_ANY on the given port.
 */
inline sockaddr_in make_bind_addr (int port)
{
    sockaddr_in addr {};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons (port);

    return addr;
}

/**
 * Build a sockaddr_in targeting the given ip and port.
 */
inline sockaddr_in make_dest_addr (const char* ip, int port)
{
    sockaddr_in dest {};
    dest.sin_family = AF_INET;
    dest.sin_port = htons (port);
    inet_pton (AF_INET, ip, &dest.sin_addr);

    return dest;
}

/**
 * Bind a socket. Returns 0 on success.
 */
inline int bind_socket (int sock, int port)
{
    sockaddr_in addr = make_bind_addr (port);
    return bind (sock, (sockaddr*) &addr, sizeof (addr));
}

/**
 * Receive a packet. Returns bytes read, or < 1 on failure.
 */
inline ssize_t receive_packet (int sock, Packet& packet)
{
    ssize_t ret = recv (sock, &packet, sizeof (packet), 0);
    
    packet.id = ntohl (packet.id);
    packet.byte_count = ntohl (packet.byte_count);

    return ret;
}

/**
 * Send a packet to a destination. Returns sendto result.
 * Adjusts packet endianness
 */
inline ssize_t send_packet (int sock, Packet& packet,
                                   const sockaddr_in& dest)
{
    packet.id = ntohl (packet.id);
    packet.byte_count = ntohl (packet.byte_count);

    size_t len = sizeof (packet.id) + sizeof (packet.byte_count)
                                    + packet.byte_count;
    std::cout << len << std::endl;
    return sendto (sock, &packet, len, 0,
                   (const sockaddr*) &dest, sizeof (dest));
}
