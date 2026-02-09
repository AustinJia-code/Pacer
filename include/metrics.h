/**
 * @file metrics.h
 * @brief Defines metrics tracked
 */

#pragma once

#include "types.h"
#include <array>
#include <string>
#include <cstdio>

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

/**
 * Emulator metrics
 */
struct EmulatorMetrics
{
    size_t fwd_data;
    size_t fwd_acks;
    size_t dropped;
};

/**
 * Terminal display with in-place line rewriting
 */
class Display
{
private:
    static constexpr size_t max_events = 10;
    std::array<std::string, max_events> events {};
    size_t head = 0;
    size_t count = 0;

public:
    /**
     * Add a line to the display
     */
    void add_event (const std::string& event)
    {
        events[head] = event;
        head = (head + 1) % max_events;
        if (count < max_events)
            ++count;
    }

    /**
     * Show display
     */
    void render (const std::string& header,
                 const std::string& stats)
    {
        std::printf ("\033[H");

        auto line = [] (const std::string& text)
        {
            std::printf ("\033[2K%s\n", text.c_str ());
        };

        line (header);
        line (stats);
        line ("");
        line ("Recent:");

        size_t start = (head + max_events - count) % max_events;
        for (size_t i = 0; i < max_events; ++i)
        {
            if (i < count)
                line ("  " + events[(start + i) % max_events]);
            else
                line ("");
        }

        std::fflush (stdout);
    }
};