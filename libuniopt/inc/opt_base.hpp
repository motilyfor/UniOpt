/**
 * @file opt_base.hpp
 * @brief Base definitions and compiler compatibility macros for optimization library.
 * 
 * Provides portable macros for restrict qualifiers, inline directives, and
 * conditional inclusion of SIMD headers (AVX2/NEON) based on architecture detection.
 * 
 * @author motilyfor
 * @email motilyfor@foxmail.com
 * @date 2026-06-12
 * @version 1.0
 * 
 * @details
 * This header defines:
 * - RESTRICT macro for pointer aliasing hints across compilers
 * - INLINE macro for forced inlining with compiler-specific attributes
 * - Architecture detection for x86_64 (AVX2) and ARM64 (NEON)
 * - Conditional inclusion of SIMD intrinsics headers
 */

#pragma once

/**
 * @def RESTRICT
 * @brief Portable restrict pointer qualifier for pointer aliasing hints.
 * 
 * Maps to `__restrict` (MSVC), `__restrict__` (GCC/Clang), or empty on unsupported compilers.
 */
#ifndef RESTRICT
#  if defined(_MSC_VER)
#    define RESTRICT __restrict
#  elif defined(__clang__) || defined(__GNUC__)
#    define RESTRICT __restrict__
#  else
#    define RESTRICT
#  endif
#endif

//---------------------------------------------------------------------
// force inline for compilers
//---------------------------------------------------------------------

/**
 * @def INLINE
 * @brief Portable force-inline macro for all supported compilers.
 * 
 * Maps to:
 * - `__inline__ __attribute__((always_inline))` for GCC 3.1+
 * - `__inline__` for older GCC
 * - `__forceinline` for MSVC
 * - `__inline` for Borland/Watcom
 * - Empty for unsupported compilers
 */
#ifndef INLINE
#ifdef __GNUC__
#if (__GNUC__ > 3) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1))
    #define INLINE         __inline__ __attribute__((always_inline))
#else
    #define INLINE         __inline__
#endif
#elif defined(_MSC_VER)
	#define INLINE __forceinline
#elif (defined(__BORLANDC__) || defined(__WATCOMC__))
    #define INLINE __inline
#else
    #define INLINE 
#endif
#endif

/**
 * @def OPT_AVX2
 * @brief Defined when x86_64 architecture is detected; enables AVX2 SIMD optimizations.
 */

/**
 * @def OPT_NEON
 * @brief Defined when ARM64 (aarch64) architecture is detected; enables NEON SIMD optimizations.
 */

#if defined(__x86_64__) || defined(_M_X64)
    #define OPT_AVX2
    #include <immintrin.h>
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define OPT_NEON
    #include <arm_neon.h>
#endif
