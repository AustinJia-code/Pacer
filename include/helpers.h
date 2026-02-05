/**
 * @file helpers.h
 * @brief Various helper functions
 */

#pragma once

#include <chrono>

/*
 * ns_t to ms_t conversion
 */
inline ms_t ns_to_ms (ns_t ns)
{
    return (ms_t) (ns / 1000000);
}

/*
 * sec_t to us_t conversion
 */
inline us_t sec_to_usec (sec_t ts)
{
    return (us_t) (ts * 1000000);
}

/**
 * Get current time in nanoseconds
 */
inline ns_t get_time_ns ()
{
    auto now = std::chrono::steady_clock::now ();
    return ns_t {std::chrono::duration_cast<std::chrono::nanoseconds> (
                    now.time_since_epoch ()).count ()};
}

/**
 * Get current time in milliseconds
 */
inline ms_t get_time_ms ()
{
    return ns_to_ms (get_time_ns ());
}
