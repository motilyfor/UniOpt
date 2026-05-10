# Test 目录说明

本目录包含针对 `libuniopt` 的若干单元/功能测试，用于验证高性能拷贝和底层工具函数的正确性与对齐要求。

**包含的测试**
- `test_cpy`：验证 `opt_cpy` 与 `std::memcpy` 在各种长度下行为一致。
- `test_clz`：验证 `opt_clz`（计数前导零）等低级工具函数。

**构建（推荐，使用仓库提供的脚本）**
在仓库根目录下：

```bash
# 使用 clang 构建单个测试目标（推荐）
./build_linux_clang.sh --target test_cpy

# 构建所有 test 目录下的目标
./build_linux_clang.sh --target test_all

# 如果使用通用脚本（已实现为将 target 映射为 GN/Ninja 标签）：
./build.sh test_cpy

# 或者，使用 GN + ninja（在 out/ 目录生成后）：
ninja -C out/linux-x64-Debug-clang //test:test_cpy
```

**本地快速编译（不依赖 GN）**
如果你只想快速在本机用 clang 编译并运行测试（便于调试）：

```bash
mkdir -p build
clang++ -std=c++23 -mavx2 -g -O0 test/test_cpy.cpp -Ilibuniopt/inc -o build/test_cpy_local
clang++ -std=c++23 -mavx2 -g -O0 test/test_clz.cpp -Ilibuniopt/inc -o build/test_clz_local

./build/test_cpy_local
./build/test_clz_local
```

**注意事项**
- 对齐要求：本库为 header-only，库内部**不做对齐检测或自动修正**。所有传入的内存缓冲区必须由调用方按需对齐（例如 AVX/AVX2 常用 32 字节对齐）。
- 编译器与指令集：若要运行使用 AVX/AVX2 路径的测试，请确保在编译时开启相应的编译器选项（例如 `-mavx2`），并在支持的 CPU 上运行。
- 不要对未对齐的缓冲区调用要求对齐的 SIMD 内建函数（如 `_mm256_store_si256`），否则会导致未定义行为或崩溃。