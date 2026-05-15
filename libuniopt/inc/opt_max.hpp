#pragma once

#include "opt_base.hpp"
#include <cstdint>

INLINE int16_t opt_max_i16(int16_t x, int16_t y)
{
	return (x > y) ? x : y;
}

INLINE int32_t opt_max_i32(int32_t x, int32_t y)
{
	return (x > y) ? x : y;
}

INLINE void opt_vec_max_i16(int16_t* RESTRICT dst, const int16_t* RESTRICT src,
							const int16_t N, std::size_t count)
{
#if defined(OPT_NEON)
	const int16x8_t limit = vdupq_n_s16(N);
	std::size_t i = 0;
	for (; i + 8 <= count; i += 8) {
		int16x8_t a = vld1q_s16(src + i);
		vst1q_s16(dst + i, vmaxq_s16(a, limit));
	}
	for (; i < count; ++i) {
		dst[i] = opt_max_i16(src[i], N);
	}
#elif defined(OPT_AVX2)
	const __m256i limit = _mm256_set1_epi16(N);
	std::size_t i = 0;
	for (; i + 16 <= count; i += 16) {
		__m256i a = _mm256_loadu_si256((const __m256i*)(src + i));
		_mm256_storeu_si256((__m256i*)(dst + i), _mm256_max_epi16(a, limit));
	}
	for (; i < count; ++i) {
		dst[i] = opt_max_i16(src[i], N);
	}
#else
	for (std::size_t i = 0; i < count; ++i) dst[i] = opt_max_i16(src[i], N);
#endif
}

INLINE void opt_vec_max_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT src,
							const int32_t N, std::size_t count)
{
#if defined(OPT_NEON)
	const int32x4_t limit = vdupq_n_s32(N);
	std::size_t i = 0;
	for (; i + 4 <= count; i += 4) {
		int32x4_t a = vld1q_s32(src + i);
		vst1q_s32(dst + i, vmaxq_s32(a, limit));
	}
	for (; i < count; ++i) {
		dst[i] = opt_max_i32(src[i], N);
	}
#elif defined(OPT_AVX2)
	const __m256i limit = _mm256_set1_epi32(N);
	std::size_t i = 0;
	for (; i + 8 <= count; i += 8) {
		__m256i a = _mm256_loadu_si256((const __m256i*)(src + i));
		_mm256_storeu_si256((__m256i*)(dst + i), _mm256_max_epi32(a, limit));
	}
	for (; i < count; ++i) {
		dst[i] = opt_max_i32(src[i], N);
	}
#else
	for (std::size_t i = 0; i < count; ++i) dst[i] = opt_max_i32(src[i], N);
#endif
}

INLINE void opt_vec_max_i16(int16_t* RESTRICT dst, const int16_t* RESTRICT src,
							const int16_t* RESTRICT limit, std::size_t count)
{
#if defined(OPT_NEON)
	std::size_t i = 0;
	for (; i + 8 <= count; i += 8) {
		int16x8_t a = vld1q_s16(src + i);
		int16x8_t b = vld1q_s16(limit + i);
		vst1q_s16(dst + i, vmaxq_s16(a, b));
	}
	for (; i < count; ++i) {
		dst[i] = opt_max_i16(src[i], limit[i]);
	}
#elif defined(OPT_AVX2)
	std::size_t i = 0;
	for (; i + 16 <= count; i += 16) {
		__m256i a = _mm256_loadu_si256((const __m256i*)(src + i));
		__m256i b = _mm256_loadu_si256((const __m256i*)(limit + i));
		_mm256_storeu_si256((__m256i*)(dst + i), _mm256_max_epi16(a, b));
	}
	for (; i < count; ++i) {
		dst[i] = opt_max_i16(src[i], limit[i]);
	}
#else
	for (std::size_t i = 0; i < count; ++i) dst[i] = opt_max_i16(src[i], limit[i]);
#endif
}

INLINE void opt_vec_max_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT src,
							const int32_t* RESTRICT limit, std::size_t count)
{
#if defined(OPT_NEON)
	std::size_t i = 0;
	for (; i + 4 <= count; i += 4) {
		int32x4_t a = vld1q_s32(src + i);
		int32x4_t b = vld1q_s32(limit + i);
		vst1q_s32(dst + i, vmaxq_s32(a, b));
	}
	for (; i < count; ++i) {
		dst[i] = opt_max_i32(src[i], limit[i]);
	}
#elif defined(OPT_AVX2)
	std::size_t i = 0;
	for (; i + 8 <= count; i += 8) {
		__m256i a = _mm256_loadu_si256((const __m256i*)(src + i));
		__m256i b = _mm256_loadu_si256((const __m256i*)(limit + i));
		_mm256_storeu_si256((__m256i*)(dst + i), _mm256_max_epi32(a, b));
	}
	for (; i < count; ++i) {
		dst[i] = opt_max_i32(src[i], limit[i]);
	}
#else
	for (std::size_t i = 0; i < count; ++i) dst[i] = opt_max_i32(src[i], limit[i]);
#endif
}