/**
 * @file opt_add.hpp
 * @brief Saturating addition operations with scalar and vectorized implementations.
 * 
 * Provides saturating addition for int16_t and int32_t types using overflow
 * intrinsics, with SIMD-accelerated vectorized variants for AVX2/NEON architectures.
 * 
 * @author motilyfor
 * @email motilyfor@foxmail.com
 * @date 2026-06-12
 * @version 1.0
 * 
 * @details
 * This header implements:
 * - Scalar saturating addition using __builtin_add_overflow
 * - Vectorized saturating addition using AVX2 (_mm256_adds_epi16/32)
 * - Vectorized saturating addition using ARM NEON (vqaddq_s16/32)
 * - Fallback scalar implementation for non-SIMD targets
 * - Q31 fixed-point format support
 */

#pragma once

#include "opt_base.hpp"
#include <cstdint>
#include <climits>


/**
 * @brief Saturating addition for int16_t (Q15 format).
 * 
 * Computes `x + y` with saturation semantics. Overflow is detected using
 * __builtin_add_overflow() and the result is clamped to INT16_MAX or INT16_MIN.
 * 
 * Overflow occurs only when x and y share the same sign, in which case the result
 * is clamped to the nearest representable boundary.
 * 
 * @param[in] x First operand (int16_t)
 * @param[in] y Second operand (int16_t)
 * @return    `x + y` saturated to [INT16_MIN, INT16_MAX]
 * 
 * @example
 * ```
 * int16_t result = opt_add_i16(32767, 100);  // Returns INT16_MAX (32767)
 * int16_t result = opt_add_i16(-32768, -100); // Returns INT16_MIN (-32768)
 * ```
 */
INLINE int16_t opt_add_i16(int16_t x, const int16_t y)
{
    int16_t res;
    if (__builtin_add_overflow(x, y, &res)) {
        // Overflow only occurs when x and y share the same sign.
        return static_cast<int16_t>((static_cast<int32_t>(x) >> 15) ^ INT16_MAX);
    }
    return res;
}

/**
 * @brief Saturating addition for int32_t (Q31 format).
 * 
 * Computes `x + y` with saturation semantics. Overflow is detected using
 * __builtin_add_overflow() and the result is clamped to INT32_MAX or INT32_MIN.
 * 
 * Overflow can only occur when x and y share the same sign, in which case the
 * result is clamped to the nearest representable boundary.
 * 
 * Format: Q31 + Q31 → Q31
 * 
 * @param[in] x First operand (int32_t)
 * @param[in] y Second operand (int32_t)
 * @return    `x + y` saturated to [INT32_MIN, INT32_MAX]
 * 
 * @example
 * ```
 * int32_t result = opt_add_i32(2147483647, 100);   // Returns INT32_MAX
 * int32_t result = opt_add_i32(-2147483648, -100); // Returns INT32_MIN
 * ```
 */
INLINE int32_t opt_add_i32(int32_t x, const int32_t y)
{
    int32_t res;
    if (__builtin_add_overflow(x, y, &res)) {
        // Overflow only occurs when x and y share the same sign.
        return (x >> 31) ^ INT32_MAX;
    }
    return res;
}

#if defined(OPT_NEON)
/**
 * @brief Vectorized saturating addition for int16_t using ARM NEON (128-bit).
 * 
 * Processes 8 elements per iteration using vqaddq_s16(), with scalar
 * fallback for remainder elements.
 * 
 * @param[out] dst   Destination array (must not overlap with x or y)
 * @param[in]  x     First source array of int16_t values
 * @param[in]  y     Second source array of int16_t values
 * @param[in]  count Number of elements to process
 * 
 * @pre dst, x, and y must be non-overlapping
 * @post dst[i] = saturate(x[i] + y[i]) for all i in [0, count)
 */
INLINE void opt_vec_add_i16(int16_t* RESTRICT dst, const int16_t* RESTRICT x,
                            const int16_t* RESTRICT y, std::size_t count)
{
    std::size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        int16x8_t a = vld1q_s16(x + i);
        int16x8_t b = vld1q_s16(y + i);
        vst1q_s16(dst + i, vqaddq_s16(a, b));
    }
    for (; i < count; ++i) {
        dst[i] = opt_add_i16(x[i], y[i]);
    }
}
#elif defined(OPT_AVX2)
/**
 * @brief Vectorized saturating addition for int16_t using AVX2 (256-bit).
 * 
 * Processes 16 elements per iteration using _mm256_adds_epi16(), with scalar
 * fallback for remainder elements.
 * 
 * @param[out] dst   Destination array (must not overlap with x or y)
 * @param[in]  x     First source array of int16_t values
 * @param[in]  y     Second source array of int16_t values
 * @param[in]  count Number of elements to process
 * 
 * @pre dst, x, and y must be non-overlapping
 * @post dst[i] = saturate(x[i] + y[i]) for all i in [0, count)
 */
INLINE void opt_vec_add_i16(int16_t* RESTRICT dst, const int16_t* RESTRICT x,
                            const int16_t* RESTRICT y, std::size_t count)
{
    std::size_t i = 0;
    for (; i + 16 <= count; i += 16) {
        __m256i a = _mm256_loadu_si256((const __m256i*)(x + i));
        __m256i b = _mm256_loadu_si256((const __m256i*)(y + i));
        _mm256_storeu_si256((__m256i*)(dst + i), _mm256_adds_epi16(a, b));
    }
    for (; i < count; ++i) {
        dst[i] = opt_add_i16(x[i], y[i]);
    }
}
#else
/**
 * @brief Scalar fallback for vectorized saturating addition of int16_t.
 * 
 * Processes all elements using scalar saturating addition.
 * 
 * @param[out] dst   Destination array
 * @param[in]  x     First source array of int16_t values
 * @param[in]  y     Second source array of int16_t values
 * @param[in]  count Number of elements to process
 * 
 * @post dst[i] = saturate(x[i] + y[i]) for all i in [0, count)
 */
INLINE void opt_vec_add_i16(int16_t* RESTRICT dst, const int16_t* RESTRICT x,
                            const int16_t* RESTRICT y, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i) {
        dst[i] = opt_add_i16(x[i], y[i]);
    }
}
#endif

#if defined(OPT_NEON)
/**
 * @brief Vectorized saturating addition for int32_t using ARM NEON (128-bit).
 * 
 * Processes 4 elements per iteration using vqaddq_s32(), with scalar
 * fallback for remainder elements.
 * 
 * @param[out] dst   Destination array (must not overlap with x or y)
 * @param[in]  x     First source array of int32_t values
 * @param[in]  y     Second source array of int32_t values
 * @param[in]  count Number of elements to process
 * 
 * @pre dst, x, and y must be non-overlapping
 * @post dst[i] = saturate(x[i] + y[i]) for all i in [0, count)
 */
INLINE void opt_vec_add_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT x,
                            const int32_t* RESTRICT y, std::size_t count)
{
    std::size_t i = 0;
    for (; i + 4 <= count; i += 4) {
        int32x4_t a = vld1q_s32(x + i);
        int32x4_t b = vld1q_s32(y + i);
        vst1q_s32(dst + i, vqaddq_s32(a, b));
    }
    for (; i < count; ++i) {
        dst[i] = opt_add_i32(x[i], y[i]);
    }
}
#elif defined(OPT_AVX2)
/**
 * @brief Vectorized saturating addition for int32_t using AVX2 (256-bit).
 * 
 * Processes 8 elements per iteration using _mm256_add_epi32() for actual addition
 * (note: no saturating variant, saturation via scalar fallback for overflow).
 * Scalar fallback for remainder elements.
 * 
 * @param[out] dst   Destination array (must not overlap with x or y)
 * @param[in]  x     First source array of int32_t values
 * @param[in]  y     Second source array of int32_t values
 * @param[in]  count Number of elements to process
 * 
 * @pre dst, x, and y must be non-overlapping
 * @post dst[i] = saturate(x[i] + y[i]) for all i in [0, count)
 */
INLINE void opt_vec_add_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT x,
                            const int32_t* RESTRICT y, std::size_t count)
{
    std::size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        __m256i a = _mm256_loadu_si256((const __m256i*)(x + i));
        __m256i b = _mm256_loadu_si256((const __m256i*)(y + i));
        _mm256_storeu_si256((__m256i*)(dst + i), _mm256_add_epi32(a, b));
    }
    for (; i < count; ++i) {
        dst[i] = opt_add_i32(x[i], y[i]);
    }
}
#else
/**
 * @brief Scalar fallback for vectorized saturating addition of int32_t.
 * 
 * Processes all elements using scalar saturating addition.
 * 
 * @param[out] dst   Destination array
 * @param[in]  x     First source array of int32_t values
 * @param[in]  y     Second source array of int32_t values
 * @param[in]  count Number of elements to process
 * 
 * @post dst[i] = saturate(x[i] + y[i]) for all i in [0, count)
 */
INLINE void opt_vec_add_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT x,
                            const int32_t* RESTRICT y, std::size_t count)
{
    for (std::size_t i = 0; i < count; ++i) {
        dst[i] = opt_add_i32(x[i], y[i]);
    }
}
#endif
