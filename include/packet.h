/**
 * @file packet.h
 * @brief Defines and handles packets
 */

#pragma once

#include "types.h"
#include <array>

static constexpr size_t MAX_PAYLOAD_BYTE_COUNT = (2 << 10);

/**
 * Defines enum for types of packets
 */
enum class PacketType : byte_t
{
    Data    = 0,
    Ack     = 1,
};

/**
 * Header all packets share
 */
struct PacketHeader
{
    PacketType type;
    id_t id;

    bool operator < (const PacketHeader& rhs) const
    {
        return this->id < rhs.id;
    } 
};

/**
 * ACKPacket
 */
struct AckPacket
{
    PacketHeader header;
};

/**
 * DataPacket header and data
 */
struct DataPacket
{
    PacketHeader header;
    size_t byte_count;
    std::array<byte_t, MAX_PAYLOAD_BYTE_COUNT> payload = {};

    bool operator < (const DataPacket& rhs) const
    {
        return this->header < rhs.header;
    }
};

/**
 * Union of packets
 */
union UnionPacket
{
    AckPacket ack_packet;
    DataPacket data_packet;
};