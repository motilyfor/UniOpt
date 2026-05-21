#pragma once
#include <cstdint>
#include "opt_base.hpp"
#include "opt_bit.hpp"

INLINE void opt_devide(int32_t& frac, int32_t& exp, int32_t x, int32_t y)
{
    if (y == 0) {
        frac = 0x7FFFFFFF; // Return max int32_t for division by zero
        exp = 0;
        return;
    }
#if defined(OPT_AVX2)


#elif defined(OPT_NEON)


#else 
    int32_t sy, sx;
    int32_t z, e;
    sx = opt_clz(x);
    x <<= sx;
    sy = opt_clz(y);
    y <<= sy;

    z = (int32_t)0xBB6872B0 - y;
    /* 4 iterations to achieve 1 LSB for reciprocal 
    */
    e = 0x40000000 - satQ31((((int64_t)y * z) + (1L << 30)) >> 31);
    e <<= 1;
    z = L_add_ll(z, satQ31((((int64_t)z * e) + (1L << 30)) >> 31));
    e = 0x40000000 - satQ31((((int64_t)y * z) + (1L << 30)) >> 31);
    e <<= 1;
    z = L_add_ll(z, satQ31((((int64_t)z * e) + (1L << 30)) >> 31));
    e = 0x40000000 - satQ31((((int64_t)y * z) + (1L << 30)) >> 31);
    e <<= 1;
    z = L_add_ll(z, satQ31((((int64_t)z * e) + (1L << 30)) >> 31));
    e = 0x40000000 - satQ31((((int64_t)y * z) + (1L << 30)) >> 31);
    e <<= 1;
    z = L_add_ll(z, satQ31((((int64_t)z * e) + (1L << 30)) >> 31));
    /* */

    frac = z;
    exp = sy - sx + 1;
#endif
}