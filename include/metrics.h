/**
 * @file metrics.h
 * @brief Defines metrics tracked
 */

#pragma once

#include "types.h"

/**
 * Sender metrics
 */
struct SenderMetrics
{
    size_t total_sent;
    size_t unique_sent;
    float mean_latency;
};

/**
 * Receiver metrics
 */
struct ReceiverMetrics
{
    size_t total_received;
    size_t unique_received;
};