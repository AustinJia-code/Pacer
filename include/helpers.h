/**
 * @file helpers.h
 * @brief Various helper functions
 */

#pragma once

#include "types.h"

/*
 * sec_t to us_t conversion
 */
inline us_t sec_to_usec (sec_t ts)
{
    return (us_t) (ts * 1000000);
}