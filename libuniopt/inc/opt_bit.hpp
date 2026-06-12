/**
 * @file opt_bit.hpp
 * @brief Bit manipulation utilities (CLZ, CTZ, FFS).
 * 
 * Provides portable bit counting and bit manipulation functions with
 * optimized implementations for various data types and SIMD architectures.
 * 
 * @author motilyfor
 * @email motilyfor@foxmail.com
 * @date 2026-06-12
 * @version 1.0
 * 
 * @details
 * This header implements:
 * - Count Leading Zeros (CLZ) for 8/16/32/64-bit integers
 * - Find First Set (FFS) - position of least significant set bit
 * - Count Trailing Zeros (CTZ) - position from least significant end
 * - Optimized variants using AVX2 and ARM NEON intrinsics
 * - Portable fallback implementations for unsupported architectures
 */

#pragma once
#include <cstdint>
#include "opt_base.hpp"

#if defined(OPT_AVX2)
    #include <immintrin.h>
#elif defined(OPT_NEON)
    #include <arm_neon.h>
#endif

/**
 * @brief Count Leading Zeros for 32-bit unsigned integer.
 * 
 * Returns the number of zero bits immediately following the sign bit
 * (most significant bit) in a 32-bit unsigned integer.
 * 
 * @param[in] x Input 32-bit unsigned integer
 * @return    Number of leading zero bits [0, 32]
 * @retval    32 if input is 0
 * @retval    0 if input is 0x80000000 or greater
 * 
 * @example
 * ```
 * opt_clz(0x00000001U) => 31
 * opt_clz(0x80000000U) => 0
 * opt_clz(0x00000000U) => 32
 * ```
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

/**
 * @brief Count Leading Zeros for 32-bit signed integer.
 * 
 * Counts leading zeros by treating the input as unsigned after
 * left-shifting to ignore the sign bit.
 * 
 * @param[in] x Input 32-bit signed integer
 * @return    Number of leading zero bits (excluding sign extension)
 * 
 * @note The sign bit is shifted out before counting, so the result
 *       represents CLZ of the absolute value's bit pattern.
 */
INLINE int32_t opt_clz(int32_t x)
{
    return opt_clz((uint32_t)x << 1); // Shift left to ignore sign bit
}

/**
 * @brief Count Leading Zeros for 64-bit unsigned integer.
 * 
 * Returns the number of zero bits immediately following the most significant bit
 * in a 64-bit unsigned integer.
 * 
 * @param[in] x Input 64-bit unsigned integer
 * @return    Number of leading zero bits [0, 64]
 * @retval    64 if input is 0
 * @retval    0 if input is 0x8000000000000000ULL or greater
 * 
 * @example
 * ```
 * opt_clz(0x0000000000000001ULL) => 63
 * opt_clz(0x8000000000000000ULL) => 0
 * opt_clz(0x0000000000000000ULL) => 64
 * ```
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
 * @brief Count Leading Zeros for 16-bit unsigned integer.
 * 
 * Returns the number of zero bits immediately following the most significant bit.
 * 
 * @param[in] x Input 16-bit unsigned integer
 * @return    Number of leading zero bits [0, 16]
 * @retval    16 if input is 0
 * 
 * @example
 * ```
 * opt_clz((uint16_t)0x0001) => 15
 * opt_clz((uint16_t)0x8000) => 0
 * opt_clz((uint16_t)0x0000) => 16
 * ```
 */
INLINE int32_t opt_clz(uint16_t x)
{
    if (x == 0) return 16;
    return opt_clz((uint32_t)x) - 16;
}

/**
 * @brief Count Leading Zeros for 8-bit unsigned integer.
 * 
 * Returns the number of zero bits immediately following the most significant bit.
 * 
 * @param[in] x Input 8-bit unsigned integer
 * @return    Number of leading zero bits [0, 8]
 * @retval    8 if input is 0
 * 
 * @example
 * ```
 * opt_clz((uint8_t)0x01) => 7
 * opt_clz((uint8_t)0x80) => 0
 * opt_clz((uint8_t)0x00) => 8
 * ```
 */
INLINE int32_t opt_clz(uint8_t x)
{
    if (x == 0) return 8;
    return opt_clz((uint32_t)x) - 24;
}

/**
 * @brief Find First Set (FFS) - Returns the 1-based index of the least significant set bit.
 * 
 * Locates the position (1-indexed from the right) of the first (least significant)
 * bit set to 1 in a 32-bit unsigned integer.
 * 
 * @param[in] x Input 32-bit unsigned integer
 * @return    1-based index of the least significant set bit
 * @retval    0 if input is 0 (no bit is set)
 * @retval    1 if the least significant bit is set (x & 1 != 0)
 * @retval    32 if only the most significant bit is set (x == 0x80000000)
 * 
 * @example
 * ```
 * opt_ffs(0x00000001U) => 1
 * opt_ffs(0x00000002U) => 2
 * opt_ffs(0x00000000U) => 0
 * opt_ffs(0x80000000U) => 32
 * ```
 */
INLINE int32_t opt_ffs(uint32_t x)
{
    if (x == 0) return 0;
    return __builtin_ffs(x);
}

/**
 * @brief Count Trailing Zeros (CTZ) - Number of zero bits from the least significant end.
 * 
 * Returns the number of zero bits following the least significant set bit
 * in a 32-bit unsigned integer. Equivalent to finding the position of the
 * least significant bit minus 1.
 * 
 * @param[in] x Input 32-bit unsigned integer
 * @return    Number of trailing zero bits [0, 32]
 * @retval    32 if input is 0
 * @retval    0 if least significant bit is set (x & 1 != 0)
 * 
 * @example
 * ```
 * opt_ctz(0x00000001U) => 0
 * opt_ctz(0x00000002U) => 1
 * opt_ctz(0x00000004U) => 2
 * opt_ctz(0x00000000U) => 32
 * ```
 * 
 * @note Related to opt_ffs: opt_ctz(x) = opt_ffs(x) - 1 for x != 0
 */
INLINE int32_t opt_ctz(uint32_t x)
{
    if (x == 0) return 32;
    return __builtin_ctz(x);
}
