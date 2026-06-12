/**
 * @file opt_mult.hpp
 * @brief Fixed-point multiplication with scalar and vectorized implementations.
 * 
 * Provides Q15 (int16_t) and Q31 (int32_t) fixed-point multiplication with
 * saturating rounding arithmetic across scalar and SIMD architectures.
 * 
 * @author motilyfor
 * @email motilyfor@foxmail.com
 * @date 2026-06-12
 * @version 1.0
 * 
 * @details
 * This header implements:
 * - Scalar Q15 fixed-point multiplication (opt_mult_q16)
 * - Scalar Q31 fixed-point multiplication (opt_mult_q31)
 * - Vectorized Q15 multiplication using AVX2/NEON
 * - Vectorized Q31 multiplication using AVX2/NEON
 * - Fallback scalar implementations for non-SIMD targets
 */

#pragma once

#include "opt_base.hpp"
#include <climits>
#include <cstdint>

/**
 * @brief Q15 fixed-point multiplication with saturation and rounding.
 * 
 * Computes (x * y) / 2^15 for Q15 format with saturating rounding arithmetic.
 * Both inputs and output are in Q15 format: a 16-bit signed value representing
 * a number in [-1.0, 1.0).
 * 
 * The multiplication is performed with full 32-bit precision internally,
 * then rounded and shifted right by 15 bits. Results saturate to INT16_MAX
 * or INT16_MIN on overflow.
 * 
 * Format: Q15 * Q15 → Q15 (with rounding and saturation)
 * 
 * @param[in] x First operand (Q15 format)
 * @param[in] y Second operand (Q15 format)
 * @return    (x * y) / 2^15 saturated to [INT16_MIN, INT16_MAX]
 * 
 * @example
 * ```
 * int16_t one = 0x7FFF;   // Q15: 1.0 - LSB
 * int16_t half = 0x4000;  // Q15: 0.5
 * int16_t result = opt_mult_q16(one, half);  // ≈ 0x3FFF (≈ 0.5)
 * ```
 * 
 * @note This function uses saturating rounding multiply-high (SRMH) instructions
 *       when available (AVX2/NEON) for efficient vectorization.
 */
INLINE int16_t opt_mult_q16(int16_t x, int16_t y)
{
#if defined(OPT_NEON)
    return vqdmulhh_s16(x, y);
#elif defined(OPT_AVX2)
    return _mm_extract_epi16(_mm_mulhrs_epi16(_mm_set1_epi16(x), _mm_set1_epi16(y)), 0);
#else
    int32_t temp = (int32_t)x * y;
    if (temp > INT16_MAX) {
        return INT16_MAX;
    }
    else if (temp < INT16_MIN) {
        return INT16_MIN;
    }
    return (int16_t)((temp + (1 << 15)) >> 16);
#endif
}

/**
 * @brief Q31 fixed-point multiplication with saturation and rounding.
 * 
 * Computes (x * y) / 2^31 for Q31 format with saturating rounding arithmetic.
 * Both inputs and output are in Q31 format: a 32-bit signed value representing
 * a number in [-1.0, 1.0).
 * 
 * The multiplication is performed with full 64-bit precision internally,
 * then rounded and shifted right by 31 bits. Results saturate to INT32_MAX
 * or INT32_MIN on overflow.
 * 
 * Format: Q31 * Q31 → Q31 (with rounding and saturation)
 * 
 * @param[in] x First operand (Q31 format)
 * @param[in] y Second operand (Q31 format)
 * @return    (x * y) / 2^31 saturated to [INT32_MIN, INT32_MAX]
 * 
 * @example
 * ```
 * int32_t one = 0x7FFFFFFF;  // Q31: 1.0 - LSB
 * int32_t half = 0x40000000; // Q31: 0.5
 * int32_t result = opt_mult_q31(one, half); // ≈ 0x3FFFFFFF (≈ 0.5)
 * ```
 * 
 * @note This function uses saturating rounding multiply-high (SRMH) instructions
 *       when available (AVX2/NEON) for efficient implementation. The fallback
 *       uses 64-bit arithmetic for full precision.
 */
INLINE int32_t opt_mult_q31(int32_t x, int32_t y)
{
#if defined(OPT_NEON)
    return vqrdmulhh_s32(x, y);
#elif defined(OPT_AVX2)
    int64_t temp = (int64_t)x * (int64_t)y;
    temp += (1LL << 30);
    temp >>= 31;
    return (int32_t)(temp > 2147483647LL ? 2147483647LL : (temp < -2147483648LL ? -2147483648LL : temp));
#else
    int64_t temp = (int64_t)x * (int64_t)y;
    temp += (1LL << 30);
    temp >>= 31;
    return (int32_t)(temp > 2147483647LL ? 2147483647LL : (temp < -2147483648LL ? -2147483648LL : temp));
#endif
}

/**
 * @brief Vectorized Q15 fixed-point multiplication using NEON/AVX2.
 * 
 * Processes multiple Q15 fixed-point multiplications in parallel using
 * vector instructions where available. Processes 8 elements per iteration
 * (NEON) or 16 elements per iteration (AVX2), with scalar fallback for
 * remainder elements.
 * 
 * Format: Q15 * Q15 → Q15 (with rounding and saturation)
 * 
 * @param[out] dst   Destination array (must not overlap with x or y)
 * @param[in]  x     First source array of Q15 values
 * @param[in]  y     Second source array of Q15 values
 * @param[in]  count Number of elements to process
 * 
 * @pre dst, x, and y must be non-overlapping
 * @post dst[i] = opt_mult_q16(x[i], y[i]) for all i in [0, count)
 */
INLINE void opt_vec_mult_q16(int16_t* RESTRICT dst, const int16_t* RESTRICT x,
                              const int16_t* RESTRICT y, std::size_t count)
{
#if defined(OPT_NEON)
    std::size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        int16x8_t a = vld1q_s16(x + i);
        int16x8_t b = vld1q_s16(y + i);
        vst1q_s16(dst + i, vqdmulhq_s16(a, b));
    }
    for (; i < count; ++i) {
        dst[i] = opt_mult_q16(x[i], y[i]);
    }
#elif defined(OPT_AVX2)
    std::size_t i = 0;
    for (; i + 16 <= count; i += 16) {
        __m256i a = _mm256_loadu_si256((const __m256i*)(x + i));
        __m256i b = _mm256_loadu_si256((const __m256i*)(y + i));
        _mm256_storeu_si256((__m256i*)(dst + i), _mm256_mulhrs_epi16(a, b));
    }
    if (i + 8 <= count) {
        __m128i a = _mm_loadu_si128((const __m128i*)(x + i));
        __m128i b = _mm_loadu_si128((const __m128i*)(y + i));
        _mm_storeu_si128((__m128i*)(dst + i), _mm_mulhrs_epi16(a, b));
        i += 8;
    }
    for (; i < count; ++i) {
        dst[i] = opt_mult_q16(x[i], y[i]);
    }
#else
    for (std::size_t i = 0; i < count; ++i) {
        dst[i] = opt_mult_q16(x[i], y[i]);
    }
#endif
}

/**
 * @brief Vectorized Q31 fixed-point multiplication using NEON/AVX2.
 * 
 * Processes multiple Q31 fixed-point multiplications in parallel using
 * vector instructions where available. Processes 4 elements per iteration
 * (NEON) or 8 elements per iteration (AVX2), with scalar fallback for
 * remainder elements.
 * 
 * For AVX2: Handles 32-bit product computation by separating even and odd
 * lane products (64-bit), then re-interleaving results for efficiency.
 * 
 * Format: Q31 * Q31 → Q31 (with rounding and saturation)
 * 
 * @param[out] dst   Destination array (must not overlap with x or y)
 * @param[in]  x     First source array of Q31 values
 * @param[in]  y     Second source array of Q31 values
 * @param[in]  count Number of elements to process
 * 
 * @pre dst, x, and y must be non-overlapping
 * @post dst[i] = opt_mult_q31(x[i], y[i]) for all i in [0, count)
 * 
 * @details
 * AVX2 implementation uses 64-bit intermediate products via:
 * - _mm256_mul_epi32 for even-indexed lanes
 * - _mm256_srli_si256 and _mm256_mul_epi32 for odd-indexed lanes
 * - Re-interleaving via shuffle and unpack operations
 */
INLINE void opt_vec_mult_q31(int32_t* RESTRICT dst, const int32_t* RESTRICT x,
                              const int32_t* RESTRICT y, std::size_t count)
{
#if defined(OPT_NEON)
    std::size_t i = 0;
    for (; i + 4 <= count; i += 4) {
        int32x4_t a = vld1q_s32(x + i);
        int32x4_t b = vld1q_s32(y + i);
        vst1q_s32(dst + i, vqrdmulhq_s32(a, b));
    }
    for (; i < count; ++i) {
        dst[i] = opt_mult_q31(x[i], y[i]);
    }
#elif defined(OPT_AVX2)
    std::size_t i = 0;
    const __m256i rnd = _mm256_set1_epi64x(1LL << 30);
    for (; i + 8 <= count; i += 8) {
        __m256i a = _mm256_loadu_si256((const __m256i*)(x + i));
        __m256i b = _mm256_loadu_si256((const __m256i*)(y + i));

        // Compute even lanes (0, 2, 4, 6)
        __m256i p_even = _mm256_mul_epi32(a, b);
        p_even = _mm256_add_epi64(p_even, rnd);
        __m256i r_even = _mm256_srli_epi64(p_even, 31);

        // Compute odd lanes (1, 3, 5, 7)
        __m256i a_odd = _mm256_srli_si256(a, 4);
        __m256i b_odd = _mm256_srli_si256(b, 4);
        __m256i p_odd = _mm256_mul_epi32(a_odd, b_odd);
        p_odd = _mm256_add_epi64(p_odd, rnd);
        __m256i r_odd = _mm256_srli_epi64(p_odd, 31);

        // Re-interleave the 32-bit results
        __m256i even32 = _mm256_shuffle_epi32(r_even, _MM_SHUFFLE(2, 0, 2, 0));
        __m256i odd32  = _mm256_shuffle_epi32(r_odd, _MM_SHUFFLE(2, 0, 2, 0));
        __m256i result = _mm256_unpacklo_epi32(even32, odd32);

        _mm256_storeu_si256((__m256i*)(dst + i), result);
    }
    for (; i < count; ++i) {
        dst[i] = opt_mult_q31(x[i], y[i]);
    }
#else
    for (std::size_t i = 0; i < count; ++i) {
        dst[i] = opt_mult_q31(x[i], y[i]);
    }
#endif
}
