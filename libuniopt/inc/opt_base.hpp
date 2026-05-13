#pragma once

// Define a portable restrict macro
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

#if defined(__x86_64__) || defined(_M_X64)
    #define OPT_AVX2
    #include <immintrin.h>
#elif defined(__aarch64__) || defined(_M_ARM64)
    #define OPT_NEON
    #include <arm_neon.h>
#endif
