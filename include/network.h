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
#include <poll.h>
#include <set>

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
 * Receive a data packet. Returns bytes read, or < 1 on failure.
 */
inline ssize_t receive_data (int sock, DataPacket& packet)
{
    packet = {};
    ssize_t ret = recv (sock, &packet, sizeof (DataPacket), 0);
    
    packet.header.id = ntohl (packet.header.id);
    packet.byte_count = ntohl (packet.byte_count);

    return ret;
}

/**
 * Send a data packet to a destination. Returns sendto result.
 * Adjusts packet endianness
 */
inline ssize_t send_data (int sock, const DataPacket& packet,
                          const sockaddr_in& dest)
{
    DataPacket out_packet {.header = {.type = PacketType::Data,
                                      .id = htonl (packet.header.id)},
                           .byte_count = htonl (packet.byte_count)};
    memcpy (out_packet.payload.begin (), packet.payload.begin (),
            packet.byte_count);

    // Only send size assigned of full allocation
    size_t len = sizeof (packet.header.id) + sizeof (packet.byte_count)
                                           + packet.byte_count;

    return sendto (sock, &out_packet, len, 0,
                   (const sockaddr*) &dest, sizeof (dest));
}

/**
 * Receive an ack packet. Returns bytes read, < 1 on failure or timeout.
 */
inline ssize_t receive_ack (int sock, AckPacket& packet, ms_t ack_timeout)
{
    packet = {};

    // Set up timeout
    pollfd pollfds[1] = {{.fd = sock, .events = POLLIN}};
    
    // Wait on socket until data available or timeout
    int ready = poll (pollfds, 1, (int) ack_timeout);
    
    if (ready < 1)
        return -1;  // Error or timeout
    
    // Get data
    ssize_t ret = recv (sock, &packet, sizeof (AckPacket), 0);
    if (ret > 0)
        packet.header.id = ntohl (packet.header.id);

    return ret;
}

/**
 * Receive all acks
 */
inline std::set<id_t> receive_all_acks (int sock, ms_t ack_timeout)
{
    std::set<id_t> ack_ids;

    pollfd pollfds[1] = {{.fd = sock, .events = POLLIN}};
    int ready = poll (pollfds, 1, (int)ack_timeout);

    if (ready < 1)
        return ack_ids;  // empty set

    while (true)
    {
        AckPacket packet{};
        ssize_t ret = recv (sock, &packet, sizeof (AckPacket), MSG_DONTWAIT);

        if (ret <= 0)
            break;  // nothing left in buffer

        packet.header.id = ntohl (packet.header.id);
        ack_ids.insert (packet.header.id);
    }

    return ack_ids;
}

/*
 * Send an ack packet to a destination. Returns sendto result.
 */
inline ssize_t send_ack (int sock, const AckPacket& packet,
                         const sockaddr_in& dest)
{
    AckPacket out_packet {.header = {.type = PacketType::Ack,
                                     .id = htonl (packet.header.id)}};
    
    return sendto (sock, &out_packet, sizeof (AckPacket), 0,
                   (const sockaddr*) &dest, sizeof (dest));
}