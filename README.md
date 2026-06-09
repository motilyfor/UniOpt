# UNIOPT

## 构建脚本总览

本项目基于 GN + Ninja，推荐使用根目录下的构建脚本进行统一构建。

### 1) 主构建脚本

脚本: `./build.sh`

作用:
- 统一处理平台/架构/编译器/构建类型参数。
- 生成 `out/<os>-<arch>-<BuildType>-<compiler>` 输出目录。
- 自动执行 `gn gen` 和 `ninja`。
- 支持按目标构建（例如 `test_cpy` 或 `//test:test_cpy`）。

常用参数:
- `--debug` / `--release`: 构建类型。
- `--os <linux|win>`: 目标系统。
- `--arch <x64|arm64>`: 目标架构。
- `--compiler <gcc|clang>`: 编译器。
- `--target <name|label>`: 构建目标（短名或完整 GN label）。
- `--clean`: 构建前清理输出目录。
- `--gc-sections`: 启用 `-ffunction-sections -fdata-sections` 和链接 `--gc-sections`，移除未引用函数/数据，减小最终产物体积。
- `--asm`: 构建成功后导出反汇编文件（源码混排）到 `out/.../asm/*.asm`。
- `-i, --interactive`: 交互式选择构建配置。

示例:

```bash
# GCC Debug 全量构建
./build.sh --debug --arch x64 --os linux --compiler gcc

# 构建指定目标
./build.sh --debug --arch x64 --os linux --compiler gcc --target test_cpy

# 构建并裁剪未引用函数
./build.sh --debug --arch x64 --os linux --compiler gcc --target test_cpy --gc-sections

# 构建并导出反汇编
./build.sh --debug --arch x64 --os linux --compiler gcc --target test_cpy --asm
```

### 2) Linux 快捷脚本

#### `./build_linux_gcc.sh`

默认等价于:

```bash
./build.sh --debug --arch x64 --os linux --compiler gcc --clean "$@"
```

用途:
- Linux + GCC 的快捷入口。
- 默认 `Debug`、`x64`、`--clean`。
- 可继续透传其他参数（如 `--target test_cpy --gc-sections --asm`）。

示例:

```bash
./build_linux_gcc.sh --target test_cpy --gc-sections --asm
```

#### `./build_linux_clang.sh`

默认等价于:

```bash
./build.sh --debug --arch x64 --os linux --compiler clang --clean "$@"
```

用途:
- Linux + Clang 的快捷入口。
- 默认 `Debug`、`x64`、`--clean`。

示例:

```bash
./build_linux_clang.sh --target test_cpy
```

### 3) 测试构建与运行脚本

脚本: `./test_linux_clang.sh`

作用:
- 先调用 `build_linux_clang.sh` 构建。
- 再在 `out/linux-x64-Debug-clang/` 下运行指定测试二进制。

参数规则:
- 无参数时默认构建并运行 `test_opt`。
- 一个及以上参数时，第一个是目标名，后续是程序运行参数。

示例:

```bash
./test_linux_clang.sh
./test_linux_clang.sh test_cpy
./test_linux_clang.sh test_cpy --help
```

### 4) 本地构建辅助脚本

脚本: `./build/local_build.sh`

作用:
- 轻量本地构建流程（同样走 GN + Ninja）。
- 适合作为脚本调试或快速试验入口。

注意:
- 日常构建推荐优先使用根目录 `build.sh`，功能更完整（例如 `--gc-sections`、`--asm`、交互模式、模块预编译等）。

## 输出目录说明

标准输出目录格式:

```text
out/<os>-<arch>-<BuildType>-<compiler>
```

常见产物:
- 可执行文件: `out/.../<target>`
- 构建日志: `out/.../build.log`
- 反汇编文件（启用 `--asm`）: `out/.../asm/*.asm`
