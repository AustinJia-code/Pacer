/**
 * @file profiles.h
 * @brief Declares specific HazardProfiles
 */
#pragma once

#include "hazards.h"
#include "effects.h"
#include <random>
#include <algorithm>

/**
 * Defines effects of environmental hazards on packet properties
 */
class HazardProfile
{        
public:
    virtual ~HazardProfile () = default;
    
    // Returns HazardProfile effects on a specific packet id
    virtual Effects get_effects (id_t id) = 0;
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

    Effects get_effects (id_t id) override
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

    Effects get_effects (id_t) override
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

    Effects get_effects (id_t id) override
    {
        return Effects
        {
            .drop = false,
            .delay = std::max (static_cast<ms_t> (dist (rng)), ms_t {0})
        };
    }
};