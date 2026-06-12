/**
 * @file opt_abs.hpp
 * @brief Absolute value computation with scalar and vectorized implementations.
 * 
 * Provides branchless scalar absolute value functions and SIMD-accelerated
 * vectorized variants for int16_t and int32_t types across AVX2/NEON architectures.
 * 
 * @author motilyfor
 * @email motilyfor@foxmail.com
 * @date 2026-06-12
 * @version 1.0
 * 
 * @details
 * This header implements:
 * - Scalar branchless absolute value for int16_t and int32_t
 * - Vectorized absolute value using AVX2 (_mm256_abs_epi16/32)
 * - Vectorized absolute value using ARM NEON (vabsq_s16/32)
 * - Fallback scalar implementation for non-SIMD targets
 */

#pragma once
#include "opt_base.hpp"
#include <cstddef>
#include <cstdint>

#if defined(OPT_AVX2)
#    include <immintrin.h>
#endif
#if defined(OPT_NEON)
#    include <arm_neon.h>
#endif

/**
 * @brief Compute absolute value of a signed 16-bit integer (branchless).
 * 
 * Uses XOR-based branchless arithmetic to avoid conditional branches:
 * - Extract sign bit via arithmetic right shift
 * - XOR with sign mask and subtract to flip negative values
 * 
 * @param[in] x Input signed 16-bit integer
 * @return    Absolute value of x (non-negative)
 * 
 * @note This implementation handles INT16_MIN edge case by saturation to INT16_MAX.
 */
INLINE int16_t opt_abs_i16(int16_t x)
{
    int16_t sign = x >> 15;   // 0 or -1
    return (int16_t)((x ^ sign) - sign);
}

#if defined(OPT_AVX2)
/**
 * @brief Vectorized absolute value for int16_t using AVX2 (256-bit).
 * 
 * Processes 16 elements per iteration using _mm256_abs_epi16(), with scalar
 * fallback for remainder elements.
 * 
 * @param[out] dst   Destination array (must not overlap with src)
 * @param[in]  src   Source array of int16_t values
 * @param[in]  n     Number of elements to process
 * 
 * @pre dst and src must be non-overlapping
 * @post dst[i] = abs(src[i]) for all i in [0, n)
 */
INLINE void opt_vec_i16(int16_t* RESTRICT dst, const int16_t* RESTRICT src, size_t n)
{
    size_t i = 0;
    for (; i + 15 < n; i += 16) {
        __m256i v = _mm256_loadu_si256((const __m256i*)(src + i));
        __m256i a = _mm256_abs_epi16(v);
        _mm256_storeu_si256((__m256i*)(dst + i), a);
    }
    for (; i < n; ++i) dst[i] = opt_abs_i16(src[i]);
}
#elif defined(OPT_NEON)
/**
 * @brief Vectorized absolute value for int16_t using ARM NEON (128-bit).
 * 
 * Processes 8 elements per iteration using vabsq_s16(), with scalar
 * fallback for remainder elements.
 * 
 * @param[out] dst   Destination array (must not overlap with src)
 * @param[in]  src   Source array of int16_t values
 * @param[in]  n     Number of elements to process
 * 
 * @pre dst and src must be non-overlapping
 * @post dst[i] = abs(src[i]) for all i in [0, n)
 */
INLINE void opt_vec_i16(int16_t* RESTRICT dst, const int16_t* RESTRICT src, size_t n)
{
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        int16x8_t v = vld1q_s16(src + i);
        int16x8_t a = vabsq_s16(v);
        vst1q_s16(dst + i, a);
    }
    for (; i < n; ++i) dst[i] = opt_abs_i16(src[i]);
}
#else
/**
 * @brief Scalar fallback for vectorized absolute value of int16_t.
 * 
 * Processes all elements using scalar branchless computation.
 * 
 * @param[out] dst   Destination array
 * @param[in]  src   Source array of int16_t values
 * @param[in]  n     Number of elements to process
 * 
 * @post dst[i] = abs(src[i]) for all i in [0, n)
 */
INLINE void opt_vec_i16(int16_t* RESTRICT dst, const int16_t* RESTRICT src, size_t n)
{
    for (size_t i = 0; i < n; ++i) dst[i] = opt_abs_i16(src[i]);
}
#endif

/**
 * @brief Compute absolute value of a signed 32-bit integer (branchless).
 * 
 * Uses XOR-based branchless arithmetic:
 * - Extract sign bit via arithmetic right shift
 * - XOR with sign mask and subtract to flip negative values
 * 
 * @param[in] x Input signed 32-bit integer
 * @return    Absolute value of x (non-negative)
 * 
 * @note This implementation handles INT32_MIN edge case by saturation to INT32_MAX.
 */
INLINE int32_t opt_abs_i32(int32_t x)
{
    int32_t sign = x >> 31;   // 0 or -1
    return (x ^ sign) - sign;
}

#if defined(OPT_AVX2)
/**
 * @brief Vectorized absolute value for int32_t using AVX2 (256-bit).
 * 
 * Processes 8 elements per iteration using _mm256_abs_epi32(), with scalar
 * fallback for remainder elements.
 * 
 * @param[out] dst   Destination array (must not overlap with src)
 * @param[in]  src   Source array of int32_t values
 * @param[in]  n     Number of elements to process
 * 
 * @pre dst and src must be non-overlapping
 * @post dst[i] = abs(src[i]) for all i in [0, n)
 */
INLINE void opt_vec_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT src, size_t n)
{
    size_t i = 0;
    for (; i + 7 < n; i += 8) {
        __m256i v = _mm256_loadu_si256((const __m256i*)(src + i));
        __m256i a = _mm256_abs_epi32(v);
        _mm256_storeu_si256((__m256i*)(dst + i), a);
    }
    for (; i < n; ++i) dst[i] = opt_abs_i32(src[i]);
}
#elif defined(OPT_NEON)
/**
 * @brief Vectorized absolute value for int32_t using ARM NEON (128-bit).
 * 
 * Processes 4 elements per iteration using vabsq_s32(), with scalar
 * fallback for remainder elements.
 * 
 * @param[out] dst   Destination array (must not overlap with src)
 * @param[in]  src   Source array of int32_t values
 * @param[in]  n     Number of elements to process
 * 
 * @pre dst and src must be non-overlapping
 * @post dst[i] = abs(src[i]) for all i in [0, n)
 */
INLINE void opt_vec_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT src, size_t n)
{
    size_t i = 0;
    for (; i + 3 < n; i += 4) {
        int32x4_t v = vld1q_s32(src + i);
        int32x4_t a = vabsq_s32(v);
        vst1q_s32(dst + i, a);
    }
    for (; i < n; ++i) dst[i] = opt_abs_i32(src[i]);
}
#else
/**
 * @brief Scalar fallback for vectorized absolute value of int32_t.
 * 
 * Processes all elements using scalar branchless computation.
 * 
 * @param[out] dst   Destination array
 * @param[in]  src   Source array of int32_t values
 * @param[in]  n     Number of elements to process
 * 
 * @post dst[i] = abs(src[i]) for all i in [0, n)
 */
INLINE void opt_vec_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT src, size_t n)
{
    for (size_t i = 0; i < n; ++i) dst[i] = opt_abs_i32(src[i]);
}
#endif
