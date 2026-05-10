#pragma once

#include "opt_base.hpp"
#include <cstdint>
#include <climits>


INLINE int32_t opt_add_i32(int32_t x, const int32_t y)
{
    int32_t res;
    if (__builtin_add_overflow(x, y, &res)) {
        return (y > 0) ? INT32_MAX : INT32_MIN;
    }
    return res;
}

INLINE int16_t opt_add_i16(int16_t x, const int16_t y)
{
    int32_t tmp = (int32_t)x + (int32_t)y;
    if (tmp > INT16_MAX) return INT16_MAX;
    if (tmp < INT16_MIN) return INT16_MIN;
    return (int16_t)tmp;
}


INLINE void opt_vec_add_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT x, const int32_t* RESTRICT y, std::size_t count)
{
#if defined(OPT_NEON)
    // Use NEON saturated add instruction vqaddq_s32
    for (std::size_t i = 0; i + 4 <= count; i += 4) {
        int32x4_t a = vld1q_s32(reinterpret_cast<const int32_t*>(x + i));
        int32x4_t b = vld1q_s32(reinterpret_cast<const int32_t*>(y + i));
        int32x4_t s = vqaddq_s32(a, b);
        vst1q_s32(dst + i, s);
    }
    // tail
    for (std::size_t i = (count / 4) * 4; i < count; ++i) {
        dst[i] = opt_add_i32(x[i], y[i]);
    }
#elif defined(OPT_AVX)
    // x86 (AVX/AVX2) has no 32-bit signed saturating-add instruction; fallback to scalar saturating add
    for (std::size_t i = 0; i < count; ++i) {
        dst[i] = opt_add_i32(x[i], y[i]);
    }
#else
    for (std::size_t i = 0; i < count; ++i) {
        dst[i] = opt_add_i32(x[i], y[i]);
    }
#endif
}