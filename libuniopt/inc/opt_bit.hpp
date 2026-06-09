#pragma once
#include <cstdint>
#include "opt_base.hpp"

#if defined(OPT_AVX2)
    #include <immintrin.h>
#elif defined(OPT_NEON)
    #include <arm_neon.h>
#endif

/**
 * @brief Count Leading Zeros for 32-bit unsigned integer
 */
INLINE int32_t opt_clz(uint32_t x)
{
    if (x == 0) return 32;
#if defined(OPT_AVX2)
    return __builtin_clz(x);
#elif defined(OPT_NEON)
    return vclz_u32(x);
#else
    // Fallback: Binary Search CLZ
    int32_t y, m, n = 0;
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
#endif
}

INLINE int32_t opt_clz(int32_t x)
{
    return opt_clz((uint32_t)x << 1); // Shift left to ignore sign bit
}

/**
 * @brief Count Leading Zeros for 64-bit unsigned integer
 */
INLINE int32_t opt_clz(uint64_t x)
{
    if (x == 0) return 64;
#if defined(OPT_AVX2)
    return __builtin_clzll(x);
#elif defined(OPT_NEON) && defined(__aarch64__)
    uint64_t result;
    __asm__("clz %0, %1" : "=r"(result) : "r"(x));
    return (int32_t)result;
#else
    uint32_t high = (uint32_t)(x >> 32);
    if (high != 0) return opt_clz(high);
    return 32 + opt_clz((uint32_t)x);
#endif
}

/**
 * @brief Count Leading Zeros for 16-bit unsigned integer
 */
INLINE int32_t opt_clz(uint16_t x)
{
    if (x == 0) return 16;
    return opt_clz((uint32_t)x) - 16;
}

/**
 * @brief Count Leading Zeros for 8-bit unsigned integer
 */
INLINE int32_t opt_clz(uint8_t x)
{
    if (x == 0) return 8;
    return opt_clz((uint32_t)x) - 24;
}

/**
 * @brief Find First Set (FFS) - Returns the 1-based index of the least significant bit set to 1.
 */
INLINE int32_t opt_ffs(uint32_t x)
{
    if (x == 0) return 0;
    return __builtin_ffs(x);
}

/**
 * @brief Count Trailing Zeros (CTZ)
 */
INLINE int32_t opt_ctz(uint32_t x)
{
    if (x == 0) return 32;
    return __builtin_ctz(x);
}
