/**
 * @file effects.h
 * @brief Defines effects a hazard can have on a packet
 */

#pragma once

#include "types.h"

/**
 * Effects a hazard can have on a packet
 */
struct Effects
{
    bool drop;
    ms_t delay;
};