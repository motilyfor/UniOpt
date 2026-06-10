#pragma once
#include <cstdint>
#include <cstdio>
#include "opt_base.hpp"
#include "opt_bit.hpp"
#include "opt_sat.hpp"
#include "opt_add.hpp"
#include "opt_mult.hpp"

INLINE void opt_divide_q31(int32_t& frac, int32_t& exp, int32_t x, int32_t y)
{
    if (y == 0) {
        frac = 0x7FFFFFFF; // Return max int32_t for division by zero
        exp = 0;
        return;
    }
// #if defined(OPT_AVX2)

// #elif defined(OPT_NEON)


// #else 
    // 1. 处理符号
    int32_t sign = (x ^ y) >> 31;
    int32_t ux = (x ^ (x >> 31)) - (x >> 31);
    int32_t uy = (y ^ (y >> 31)) - (y >> 31);

    // 2. 规格化 (Normalize)
    int32_t sx = opt_clz(ux);
    int32_t sy = opt_clz(uy);
    ux <<= sx;
    uy <<= sy;

    // 3. 计算 1/uy 的初值 (Newton-Raphson initial guess)
    // 对于 uy \in [0.5, 1.0)，使用线性近似: z(q30) =  2.9282 - 2 * uy 
    int32_t z = (int32_t)0xBB67A0F9 - uy;

    /* 4 iterations to achieve 1 LSB for reciprocal 
    */
    int32_t e; // q30
    e = 0x40000000 - opt_sat_q31((((int64_t)uy * z) + (1L << 30)) >> 31);
    e <<= 1;
    z = opt_add_i32(z, opt_sat_q31((((int64_t)z * e) + (1L << 30)) >> 31));

    e = 0x40000000 - opt_sat_q31((((int64_t)uy * z) + (1L << 30)) >> 31);
    e <<= 1;
    z = opt_add_i32(z, opt_sat_q31((((int64_t)z * e) + (1L << 30)) >> 31));

    e = 0x40000000 - opt_sat_q31((((int64_t)uy * z) + (1L << 30)) >> 31);
    e <<= 1;
    z = opt_add_i32(z, opt_sat_q31((((int64_t)z * e) + (1L << 30)) >> 31));

    /**
     * @note Newton-Raphson typically achieves Q31 precision with 3 iterations.
     *       If precision remains insufficient for edge cases, uncomment the
     *       following 4th iteration.
     */
#if 0
    e = 0x40000000 - opt_sat_q31((((int64_t)uy * z) + (1L << 30)) >> 31);
    e <<= 1;
    z = opt_add_i32(z, opt_sat_q31((((int64_t)z * e) + (1L << 30)) >> 31));
#endif

    frac = opt_mult_q31(z, ux); // x * (1/y)
    frac = (frac ^ (sign >> 31)) - (sign >> 31);
    exp = sy - sx + 1;
// #endif
}