#pragma once
#include <cstdint>
#include "opt_clz.hpp"

inline int32_t opt_devide(int32_t a, int32_t b) {
    // Constants
    const int32_t InitialQ25 = 0x05DB3D08;  // 2.9282 in Q25 format (0x05DB3D08)
    const int FracBits = 24;                // Fixed-point fractional bits
    const int32_t OneQ24 = 1 << FracBits;   // 1.0 in Q24 = 2^24
    
    // Handle sign: division sign = sign(a) XOR sign(b)
    int signA = (a < 0) ? -1 : 1;
    int signB = (b < 0) ? -1 : 1;
    int resultSign = signA * signB;
    
    // Work with absolute values
    int32_t aAbs = (a < 0) ? -a : a;
    int32_t bAbs = (b < 0) ? -b : b;
    
    // Step 1: Find normalization shift to keep b in a good range
    // Count leading zeros to normalize b toward [2^30, 2^31)
    int clzB = opt_clz(bAbs);  // Leading zeros in bAbs
    int32_t bNormalized = bAbs << clzB;
    int normShift = clzB;
    
    // Step 2: Initial reciprocal estimate x^(0)
    // x^(0) ≈ 2.9282 / sqrt(2 * bNormalized)
    // Simplified heuristic: scale by normalization
    int32_t x = InitialQ25 - (bAbs / 2);
    
}