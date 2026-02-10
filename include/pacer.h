/**
 * @file pacer.h
 * @brief Token bucket rate limiter with inter-packet spacing
 */

#pragma once

#include "types.h"
#include "helpers.h"
#include <algorithm>

/**
 * Token bucket for rate limiting and burst suppression.
 */
class TokenBucket
{
private:
    double tokens;
    double capacity;
    double rate;
    ms_t last_refill;

public:
    /**
     * rate:     tokens/sec  -> average send rate
     * capacity: max tokens  -> max burst size
     */
    TokenBucket (double rate, double capacity)
        : tokens (capacity), capacity (capacity), rate (rate),
          last_refill (get_time_ms ()) {}

    /**
     * Add as many tokens back as possible
     */
    void refill ()
    {
        ms_t now = get_time_ms ();
        double elapsed_sec = (now - last_refill) / 1000.0;
        tokens = std::min (tokens + elapsed_sec * rate, capacity);
        last_refill = now;
    }

    /**
     * Return if tokens available to consume
     */
    bool try_consume ()
    {
        refill ();

        if (tokens < 1.0)
            return false;

        tokens -= 1.0;
        return true;
    }

    /**
     * Get number of tokens available
     */
    double available () const { return tokens; }
};
