#pragma once

#include "opt_base.hpp"
#include <cstdint>
#include <climits>

INLINE int16_t opt_sub_i16(int16_t x, const int16_t y)
{
    int16_t res;
    if (__builtin_sub_overflow(x, y, &res)) {
        // Overflow only occurs when x and y have different signs.
        return static_cast<int16_t>((static_cast<int32_t>(x) >> 15) ^ INT16_MAX);
    }
    return res;
}

INLINE int32_t opt_sub_i32(int32_t x, const int32_t y)
{                   
    int32_t res;
    if (__builtin_sub_overflow(x, y, &res)) {
        // Overflow only occurs when x and y have different signs.
        return (x >> 31) ^ INT32_MAX;
    }
    return res;
}

INLINE void opt_vec_sub_i16(int16_t* RESTRICT dst, const int16_t* RESTRICT x,
                            const int16_t* RESTRICT y, std::size_t count)
{
#if defined(OPT_NEON)
    std::size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        int16x8_t a = vld1q_s16(x + i);
        int16x8_t b = vld1q_s16(y + i);
        vst1q_s16(dst + i, vqsubq_s16(a, b));
    }
    for (; i < count; ++i) {
        dst[i] = opt_sub_i16(x[i], y[i]);
    }
#elif defined(OPT_AVX2)
    std::size_t i = 0;
    for (; i + 16 <= count; i += 16) {
        __m256i a = _mm256_loadu_si256((const __m256i*)(x + i));
        __m256i b = _mm256_loadu_si256((const __m256i*)(y + i));
        _mm256_storeu_si256((__m256i*)(dst + i), _mm256_subs_epi16(a, b));
    }
    for (; i < count; ++i) {
        dst[i] = opt_sub_i16(x[i], y[i]);
    }
#else
    for (std::size_t i = 0; i < count; ++i) {
        dst[i] = opt_sub_i16(x[i], y[i]);
    }
#endif
}

INLINE void opt_vec_sub_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT x,
                            const int32_t* RESTRICT y, std::size_t count)
{
#if defined(OPT_NEON)
    std::size_t i = 0;
    for (; i + 4 <= count; i += 4) {
        int32x4_t a = vld1q_s32(x + i);
        int32x4_t b = vld1q_s32(y + i);
        vst1q_s32(dst + i, vqsubq_s32(a, b));
    }
    for (; i < count; ++i) {
        dst[i] = opt_sub_i32(x[i], y[i]);
    }
#elif defined(OPT_AVX2)
    std::size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        __m256i a = _mm256_loadu_si256((const __m256i*)(x + i));
        __m256i b = _mm256_loadu_si256((const __m256i*)(y + i));
        _mm256_storeu_si256((__m256i*)(dst + i), _mm256_sub_epi32(a, b));
    }
    for (; i < count; ++i) {
        dst[i] = opt_sub_i32(x[i], y[i]);
    }
#else
    for (std::size_t i = 0; i < count; ++i)
        dst[i] = opt_sub_i32(x[i], y[i]);
#endif
}