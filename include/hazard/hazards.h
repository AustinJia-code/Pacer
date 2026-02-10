/**
 * @file profiles.h
 * @brief Declares specific HazardProfiles
 */

#pragma once

#include "hazards.h"
#include "effects.h"
#include "helpers.h"
#include <random>
#include <algorithm>
#include <packet.h>

/**
 * Defines effects of environmental hazards on packet properties
 */
class HazardProfile
{        
public:
    virtual ~HazardProfile () = default;
    
    // Returns HazardProfile effects on a specific packet id
    virtual Effects get_effects (PacketType type, id_t id) = 0;
};

/**
 * Forward declarations, defined in src/hazard/profiles.cpp
 */
/**
 * Randomly drops some packets
 */
class RandomLoss : public HazardProfile
{
private:
    std::default_random_engine rng;
    std::bernoulli_distribution drop_dist;

public:
    /**
     * Default 5% loss
     */
    RandomLoss (float loss_ratio = 0.05, unsigned int seed = 0)
        : drop_dist (loss_ratio), rng (seed) {}

    Effects get_effects (PacketType type, id_t id) override
    {
        return Effects
        {
            .drop  = drop_dist (rng),
            .delay = ms_t {0}
        };
    }
};

/**
 * Randomly increments loss count, randomly drops loss count contiguous packets
 */
class BurstLoss : public HazardProfile
{
private:
    std::default_random_engine rng;
    std::bernoulli_distribution loss_dist;
    std::bernoulli_distribution burst_start_dist;

    size_t drop_count = 0;
    bool dropping = false;

public:
    /**
     * Default 5% loss, 0.5% drop chance
     */
    BurstLoss (float loss_ratio = 0.05, float drop_chance = 0.005,
               unsigned int seed = 0)
        : loss_dist (loss_ratio), burst_start_dist (drop_chance), rng (seed) {}

    Effects get_effects (PacketType type, id_t id) override
    {
        Effects effects {};
        effects.delay = ms_t {0};

        if (loss_dist (rng))
            ++drop_count;

        // Sample to start burst drop
        if (!dropping && burst_start_dist (rng) && drop_count > 0)
            dropping = true;

        // Burst drop
        if (dropping)
        {
            effects.drop = true;
            if (--drop_count == 0)
                dropping = false;
        }

        return effects;
    }
};

/**
 * Simulates a router with a small packet buffer.
 * Packets arriving when buffer is full are tail-dropped.
 * Buffer drains at a fixed rate (link bandwidth).
 *
 * Punishes bursty senders: a burst of 10 into a buffer of 6 = 4 dropped.
 * A paced burst of 5 fits cleanly.
 */
class ShallowBuffer : public HazardProfile
{
private:
    size_t capacity;
    size_t occupied = 0;
    double drain_rate;
    ms_t last_drain;

public:
    ShallowBuffer (size_t capacity = 5, double drain_rate = 60.0)
        : capacity (capacity), drain_rate (drain_rate),
          last_drain (get_time_ms ()) {}

    Effects get_effects (PacketType type, id_t id) override
    {
        // Acks are tiny, only buffer data (forward path)
        if (type == PacketType::Ack)
            return Effects {.drop = false, .delay = ms_t {0}};

        ms_t now = get_time_ms ();
        if (now > last_drain)
        {
            double elapsed_sec = (now - last_drain) / 1000.0;
            size_t drained = (size_t) (elapsed_sec * drain_rate);
            if (drained > occupied) drained = occupied;
            occupied -= drained;
            last_drain = now;
        }

        if (occupied >= capacity)
            return Effects {.drop = true, .delay = ms_t {0}};

        ++occupied;
        return Effects {.drop = false, .delay = ms_t {0}};
    }
};

/**
 * Applies random delay
 */
class RandomJitter : public HazardProfile
{
private:
    std::default_random_engine rng;
    std::normal_distribution<double> dist;

    ms_t min_delay;
    ms_t max_delay;

public:
    RandomJitter (ms_t mean_delay = 100, ms_t std_delay = 80,
                  unsigned int seed = 0)
        : dist (mean_delay, std_delay), rng (seed) {}

    Effects get_effects (PacketType type, id_t id) override
    {
        return Effects
        {
            .drop = false,
            .delay = std::max (static_cast<ms_t> (dist (rng)), ms_t {0})
        };
    }
};