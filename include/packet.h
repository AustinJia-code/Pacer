/**
 * @file packet.h
 * @brief Defines and handles packets
 */

#pragma once

#include "types.h"

static constexpr size_t MAX_PAYLOAD_BYTE_COUNT = (2 << 10);

/**
 * Packet header and data
 */
struct Packet
{
    id_t id;
    size_t byte_count;
    byte_t payload[MAX_PAYLOAD_BYTE_COUNT];
};