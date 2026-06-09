#pragma once

#include "opt_base.hpp"
#include <cstdint>

/**
 * @brief Saturate an @c int64_t value to @c int32_t (Q31).
 * @param x Input 64-bit integer.
 * @return Saturated 32-bit integer.
 */
INLINE int32_t opt_sat_q31(int64_t x)
{
    if (x > (int64_t)0x000000007FFFFFFFLL) {
        x = 0x7FFFFFFF;
    }
    if (x < (int64_t)0xFFFFFFFF80000000LL) {
        x = 0x80000000;
    }
    return (int32_t)x;
}
