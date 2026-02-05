/**
 * @file packet.h
 * @brief Defines and handles packets
 */

#pragma once

#include "types.h"

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
    byte_t payload[MAX_PAYLOAD_BYTE_COUNT];
};

/**
 * Union of packets
 */
union UnionPacket
{
    AckPacket ack_packet;
    DataPacket data_packet;
};