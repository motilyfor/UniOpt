#pragma once
#include <cstdint>
#include "opt_base.hpp"

/**
 * @brief 优化的范围检查算法 (Branchless Range Check)
 * 
 * 核心原理：
 * 利用无符号数减法溢出特性：(unsigned)(val - start) <= (unsigned)(end - start)
 * 将两次边界检查合并为一次比较，减少分支跳转，提升 CPU 流水线效率。
 */

static INLINE bool opt_is_in_range(uint32_t val, uint32_t start, uint32_t end) {
    return (uint32_t)(val - start) <= (uint32_t)(end - start);
}

static INLINE bool opt_is_in_range(int32_t val, int32_t start, int32_t end) {
    return (uint32_t)(val - start) <= (uint32_t)(end - start);
}

static INLINE bool opt_is_in_range(uint64_t val, uint64_t start, uint64_t end) {
    return (uint64_t)(val - start) <= (uint64_t)(end - start);
}

static INLINE bool opt_is_in_range(int64_t val, int64_t start, int64_t end) {
    return (uint64_t)(val - start) <= (uint64_t)(end - start);
}

static INLINE bool opt_is_in_range(uint16_t val, uint16_t start, uint16_t end) {
    return (uint16_t)(val - start) <= (uint16_t)(end - start);
}

static INLINE bool opt_is_in_range(int16_t val, int16_t start, int16_t end) {
    return (uint16_t)(val - start) <= (uint16_t)(end - start);
}

static INLINE bool opt_is_in_range(uint8_t val, uint8_t start, uint8_t end) {
    return (uint8_t)(val - start) <= (uint8_t)(end - start);
}

static INLINE bool opt_is_in_range(int8_t val, int8_t start, int8_t end) {
    return (uint8_t)(val - start) <= (uint8_t)(end - start);
}
