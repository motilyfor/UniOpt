---
name: opt-conventions
version: 1.0
scope: workspace
short_description: "UniOpt 函数命名与向量化实现规范（AVX2 / NEON）"
---

# UniOpt: 函数命名与向量化实现规范

目的：为团队提供统一的函数命名约定和实现模板，确保所有算子在标量与向量（AVX2 / NEON）路径上具有一致的接口与实现风格，便于审查、测试与自动化替换。

适用范围：本仓库函数实现，尤其是定点/整型算子（i16/i32/q31/q15 等）。将该文件放在 `.github/skills/opt-conventions/SKILL.md`，团队可作为参考。

## 关键命名约定
- 标量函数：`opt_<op>_<type>`，例如 `opt_add_i32`、`opt_abs_i16`、`opt_div_q31`。
- 向量/批量函数：`opt_vec_<op>_<type>`，例如 `opt_vec_add_i32`、`opt_vec_abs_i16`、`opt_vec_div_q31`。
- Q 格式后缀：使用 `q31`, `q15` 等明确定点格式（小写）。

示例：
- 单元素绝对值（标量）：`int32_t opt_abs_i32(int32_t x)`
- 批量绝对值（向量化）：`void opt_vec_abs_i32(int32_t* dst, const int32_t* src, size_t n)`

## 针对每个算子要求的实现分支
每个 `opt_<...>` 算子在实现时，应提供以下分支（按条件编译）：
1. 标量实现（必需）
2. AVX2 向量化实现（`#if defined(OPT_AVX2)`）——优先使用 `_mm256_*` intrinsics
3. NEON 向量化实现（`#elif defined(OPT_NEON)`）——优先使用 `v*q_*` intrinsics
4. 回退路径（未定义 SIMD 时）使用标量循环或逐元素调用标量函数

所有向量化函数必须：
- 处理任意长度 `n`，对齐优化（aligned load/store）可选，但实现须使用不对齐加载/存储（`loadu`）或在文档中明确要求对齐；
- 处理尾部（tail）元素，使用标量回退或单独处理尾部掩码；
- 保持与标量函数在数值和饱和行为上的一致性（对定点算子尤其重要）。

## 模板示例：`opt_vec_abs_i32`

```cpp
INLINE int32_t opt_abs_i32(int32_t x) {
  int32_t sign = x >> 31;
  return (x ^ sign) - sign;
}

#if defined(OPT_AVX2)
INLINE void opt_vec_abs_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT src, size_t n) {
  size_t i = 0;
  for (; i + 7 < n; i += 8) {
    __m256i v = _mm256_loadu_si256((const __m256i*)(src + i));
    __m256i a = _mm256_abs_epi32(v);
    _mm256_storeu_si256((__m256i*)(dst + i), a);
  }
  for (; i < n; ++i) dst[i] = opt_abs_i32(src[i]);
}
#elif defined(OPT_NEON)
INLINE void opt_vec_abs_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT src, size_t n) {
  size_t i = 0;
  for (; i + 3 < n; i += 4) {
    int32x4_t v = vld1q_s32(src + i);
    int32x4_t a = vabsq_s32(v);
    vst1q_s32(dst + i, a);
  }
  for (; i < n; ++i) dst[i] = opt_abs_i32(src[i]);
}
#else
INLINE void opt_vec_abs_i32(int32_t* RESTRICT dst, const int32_t* RESTRICT src, size_t n) {
  for (size_t i = 0; i < n; ++i) dst[i] = opt_abs_i32(src[i]);
}
#endif
```

## Q31 / 定点算子注意事项
- 中间乘法应使用 `int64_t` 做扩展，之后按需做位移与舍入；
- 使用 `opt_sat_q31` 或类似饱和函数保证行为一致；
- 对于倒数/除法类算法（Newton-Raphson）：向量化时保持迭代次数与舍入策略一致，确保数值稳定性。

## 测试与验证
- 每个 `opt_vec_*` 函数必须配套单元测试：
  - 逐元素（n=1..8）与大规模（n=1000）覆盖；
  - 包含未对齐地址和尾部长度（n%SIMD != 0）；
  - 比较标量实现与向量实现的输出一致性（数值误差容限需在文档中声明）。

## 代码审查清单（PR 模板）
- [ ] 是否提供标量实现？
- [ ] 是否提供 AVX2 或 NEON 实现（至少一项）？
- [ ] 向量实现是否处理尾部？
- [ ] 是否包含单元测试覆盖对齐/未对齐与尾部场景？
- [ ] 是否在文档中说明对齐要求和 ABI 依赖？

## 示例 prompts（供 Copilot 使用）
- "为 `opt_divide_q31` 生成 AVX2 与 NEON 向量化实现，保留现有标量接口，并处理尾部长度。"
- "请生成 `opt_vec_mul_q31` 的单测，覆盖对齐/未对齐和尾部长度。"

---

如果你希望我把该 `SKILL.md` 复制到用户范围（个人 prompts 目录）或更新为更严格的 frontmatter（例如 `applyTo` globs），告诉我目标位置与触发关键字，我会继续。