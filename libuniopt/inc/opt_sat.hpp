/**
 * @file opt_sat.hpp
 * @brief Saturation utilities for fixed-point arithmetic.
 * 
 * Provides saturation functions to clamp values to representable ranges
 * for fixed-point arithmetic operations, particularly for Q31 format.
 * 
 * @author motilyfor
 * @email motilyfor@foxmail.com
 * @date 2026-06-12
 * @version 1.0
 * 
 * @details
 * This header implements:
 * - Saturation of 64-bit values to 32-bit Q31 range
 * - Clamping to [INT32_MIN, INT32_MAX]
 * - Used in intermediate calculations for multiplication and division
 */

#pragma once

#include "opt_base.hpp"
#include <cstdint>

/**
 * @brief Saturate a 64-bit integer to 32-bit (Q31) range.
 * 
 * Clamps a 64-bit signed integer to the range [INT32_MIN, INT32_MAX].
 * Used to prevent overflow in intermediate calculations during fixed-point
 * multiplication and division operations.
 * 
 * The function performs range checking and returns:
 * - INT32_MAX (0x7FFFFFFF) if x > INT32_MAX
 * - INT32_MIN (0x80000000) if x < INT32_MIN
 * - x cast to int32_t otherwise
 * 
 * @param[in] x Input 64-bit signed integer
 * @return    x clamped to [INT32_MIN, INT32_MAX]
 * 
 * @example
 * ```
 * int64_t large = 0x0000000100000000LL;  // 2^32
 * int32_t sat = opt_sat_q31(large);      // Returns INT32_MAX (0x7FFFFFFF)
 * 
 * int64_t small = -0x0000000200000000LL; // -2^33
 * int32_t sat = opt_sat_q31(small);      // Returns INT32_MIN (0x80000000)
 * 
 * int64_t normal = 0x0000000040000000LL; // Within range
 * int32_t sat = opt_sat_q31(normal);     // Returns 0x40000000
 * ```
 * 
 * @note This function is primarily used internally by fixed-point arithmetic
 *       routines (multiplication, division, addition) to handle intermediate
 *       results that may exceed 32-bit range.
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
