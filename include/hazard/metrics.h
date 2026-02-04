/**
 * @file metrics.h
 * @brief Defines metrics tracked for each HazardProfile
 */

#pragma once

#include "types.h"

/**
 * Metrics tracked for each HazardProfile
 */
struct Metrics
{
    float goodput;
    float throughput;
    float retransmission_factor;
    bool integrity;
    size_t stall_count;
};