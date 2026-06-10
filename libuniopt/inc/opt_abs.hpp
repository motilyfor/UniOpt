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

INLINE int16_t opt_abs_i16(int16_t x)
{
    int16_t sign = x >> 15;   // 0 or -1
    return (int16_t)((x ^ sign) - sign);
}

#if defined(OPT_AVX2)
// Vectorized absolute for i16 (process 16 elements per iteration)
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
// NEON: vectorized absolute for i16 (process 8 elements per iteration)
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
INLINE void opt_vec_i16(int16_t* RESTRICT dst, const int16_t* RESTRICT src, size_t n)
{
    for (size_t i = 0; i < n; ++i) dst[i] = opt_abs_i16(src[i]);
}
#endif

// Scalar branchless absolute for int32_t
INLINE int32_t opt_abs_i32(int32_t x)
{
    int32_t sign = x >> 31;   // 0 or -1
    return (x ^ sign) - sign;
}

#if defined(OPT_AVX2)
// Vectorized absolute for i32 (process 8 elements per iteration)
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
// NEON: vectorized absolute for i32 (process 4 elements per iteration)
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
INLINE void opt_vec_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT src, size_t n)
{
    for (size_t i = 0; i < n; ++i) dst[i] = opt_abs_i32(src[i]);
}
#endif