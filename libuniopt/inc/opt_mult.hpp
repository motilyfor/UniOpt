#pragma once

#include "opt_base.hpp"
#include <climits>
#include <cstdint>

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
