/**
 * @file helpers.h
 * @brief Various helper functions
 */

#include "types.h"

/*
 * sec_t to usec_t conversion
 */
static inline usec_t sec_to_usec (sec_t ts)
{
    return (usec_t) (ts * 1000000);
}