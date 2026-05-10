#pragma once
#include <cstdint>

#include "opt_base.hpp"

static INLINE int opt_clz(uint32_t x)
{
    int y, m, n = 0;

    y = -(x >> 16);
    m = (y >> 16) & 16u;
    n = 16 - m;
    x >>= m;

    y = x - 0x100;
    m = (y >> 16) & 8u;
    n += m;
    x <<= m;

    y = x - 0x1000;
    m = (y >> 16) & 4u;
    n += m;
    x <<= m;

    y = x - 0x4000;
    m = (y >> 16) & 2u;
    n += m;
    x <<= m;

    y = x >> 14;
    m = y & ~(y >> 1);
    return n + 2 - m;
}

static INLINE int opt_clz(int32_t x)
{
    int y, m, n = 0;

    y = -(x >> 16);
    m = (y >> 16) & 16u;
    n = 16 - m;
    x >>= m;

    y = x - 0x100;
    m = (y >> 16) & 8u;
    n += m;
    x <<= m;

    y = x - 0x1000;
    m = (y >> 16) & 4u;
    n += m;
    x <<= m;

    y = x - 0x4000;
    m = (y >> 16) & 2u;
    n += m;
    x <<= m;

    y = x >> 14;
    m = y & ~(y >> 1);
    return n + 2 - m;
}
