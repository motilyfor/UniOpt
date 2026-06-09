#!/bin/bash

# 遇到错误立即退出，管道中任意命令失败也退出
set -eo pipefail

# 获取脚本所在目录作为项目根目录
ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# 默认配置
BUILD_TYPE="Debug"
TARGET_OS="linux"
TARGET_CPU="x64"
COMPILER="gcc"
find_target_label() {
    # Sets global NINJA_TARGET on success and returns 0.
    if [[ -z "${BUILD_TARGET}" ]]; then
        return 1
    fi
    if [[ "${BUILD_TARGET}" == *"/"* || "${BUILD_TARGET}" == *":"* ]]; then
        NINJA_TARGET="${BUILD_TARGET}"
        return 0
    fi

    local name="${BUILD_TARGET}"
    local -a matches
    # Search for canonical target declarations in BUILD.gn
    mapfile -t matches < <(grep -Rsl --include=BUILD.gn -E "^(\s)*(executable|static_library|shared_library|source_set|group)\\(\\s*\"${name}\"\\s*\\)" "$ROOT_DIR" --exclude-dir=.git --exclude-dir=out --exclude-dir=.vscode || true)

    # Fallback to any occurrence of the quoted name in BUILD.gn
    if [ "${#matches[@]}" -eq 0 ]; then
        mapfile -t matches < <(grep -Rsl --include=BUILD.gn -F "\"${name}\"" "$ROOT_DIR" --exclude-dir=.git --exclude-dir=out --exclude-dir=.vscode || true)
    fi

    if [ "${#matches[@]}" -eq 0 ]; then
        return 1
    fi

    local chosen
    if [ "${#matches[@]}" -gt 1 ]; then
        echo "Multiple BUILD.gn matches for target '${name}':"
        local i=1
        for f in "${matches[@]}"; do
            echo "  ${i}) ${f}"
            i=$((i+1))
        done
        if [ -t 0 ]; then
            read -p "Select index to use [1-${#matches[@]}] (default 1): " sel
            sel=${sel:-1}
            sel=$((sel-1))
            chosen="${matches[$sel]}"
        else
            chosen="${matches[0]}"
            echo "Non-interactive: selecting first match: ${chosen}" >&2
        fi
    else
        chosen="${matches[0]}"
    fi

    # compute label from file path
    local rel
    rel=${chosen#${ROOT_DIR}/}
    local dir
    dir=$(dirname "$rel")
    if [ "$dir" = "." ] || [ -z "$dir" ]; then
        NINJA_TARGET="//:${name}"
    else
        NINJA_TARGET="//${dir}:${name}"
    fi
    return 0
}
# 帮助信息
function show_help {
    echo "Usage: ./build.sh [options]"
    echo "Options:"
    echo "  -i, --interactive 交互式配置模式"
    echo "  --debug           构建 Debug 版本 (默认)"
    echo "  --release         构建 Release 版本"
    echo "  --arch <arch>     目标架构 (x64, arm64). 默认: $TARGET_CPU"
    echo "  --os <os>         目标操作系统 (linux, win). 默认: linux"
    echo "  --compiler <cc>   编译器 (gcc, clang). 默认: gcc"
    echo "  --target <name>   构建目标或具体子目标（例如: test, test_cpy, //test:test_cpy）。若不指定则构建默认顶层目标"
    echo "  --clean           构建前清理输出目录"
    echo "  --gc-sections     启用函数/数据分段并在链接时移除未引用代码，缩小产物体积"
    echo "  --asm             构建成功后导出反汇编文件到 out/.../asm，用于和源码比对"
    echo "  --help            显示帮助信息"
}

function export_asm_artifacts {
    local out_dir="$1"
    local ninja_target="$2"
    local asm_dir="${out_dir}/asm"
    local -a artifacts=()

    if ! command -v objdump >/dev/null 2>&1; then
        echo "警告: 未找到 objdump，跳过反汇编导出。"
        return
    fi

    mkdir -p "${asm_dir}"

    if [[ -n "${ninja_target}" && "${ninja_target}" == *:* ]]; then
        local rel_bin="${ninja_target/://}"
        if [ -f "${out_dir}/${rel_bin}" ] && [ -x "${out_dir}/${rel_bin}" ]; then
            artifacts+=("${out_dir}/${rel_bin}")
        fi
    fi

    if [ "${#artifacts[@]}" -eq 0 ]; then
        while IFS= read -r f; do
            artifacts+=("$f")
        done < <(find "${out_dir}" -type f -perm -u+x \
            ! -name "*.so" \
            ! -name "*.a" \
            ! -path "${out_dir}/obj/*" \
            ! -path "${out_dir}/asm/*")
    fi

    if [ "${#artifacts[@]}" -eq 0 ]; then
        echo "未找到可导出反汇编的可执行产物。"
        return
    fi

    echo "正在导出反汇编文件到: ${asm_dir}"
    for bin in "${artifacts[@]}"; do
        local rel
        rel=$(realpath --relative-to="${out_dir}" "${bin}")
        local safe_name
        safe_name=$(echo "${rel}" | sed 's#[/ ]#_#g')
        local asm_file="${asm_dir}/${safe_name}.asm"

        # -S 将源码与汇编混排，-l 显示源代码行号，-C 做 C++ 符号反修饰。
        objdump -d -S -l -C -M intel "${bin}" > "${asm_file}"
        echo "  ${rel} -> ${asm_file}"
    done

    echo "提示: 为了更完整的源码行信息，建议使用 --debug 构建。"
}

function precompile_clang_modules {
    if [ "$COMPILER" != "clang" ] || [ "$TARGET_OS" != "linux" ]; then
        return
    fi
    # Only precompile modules for test-related targets (if a target is specified
    # and it does not reference test, skip precompilation).
    if [ -n "$BUILD_TARGET" ] && [[ "$BUILD_TARGET" != *test* ]]; then
        return
    fi

    echo "正在预编译 C++23 Modules (clang)..."

    local clang_args=("-std=c++23" "-fPIC" "-pthread")
    if [ "$TARGET_CPU" == "x64" ]; then
        clang_args+=("-mavx" "-DARCH_X86_64")
    elif [ "$TARGET_CPU" == "arm64" ]; then
        clang_args+=("-march=armv8-a+fp+simd" "-DARCH_ARM64")
    fi

    if [ "$BUILD_TYPE" == "Debug" ]; then
        clang_args+=("-O0" "-g")
    else
        clang_args+=("-O3")
    fi

    mapfile -t module_sources < <(
        grep -Rsl --include="*.cpp" --include="*.cppm" --include="*.ixx" \
            '^[[:space:]]*export[[:space:]]\+module[[:space:]]\+[A-Za-z_][A-Za-z0-9_.:]*[[:space:]]*;' \
            "$ROOT_DIR" --exclude-dir=.git --exclude-dir=out --exclude-dir=.vscode
    )

    if [ "${#module_sources[@]}" -eq 0 ]; then
        echo "未发现 module interface，跳过预编译。"
        return
    fi

    for src in "${module_sources[@]}"; do
        local module_name
        module_name=$(sed -nE 's/^[[:space:]]*export[[:space:]]+module[[:space:]]+([A-Za-z_][A-Za-z0-9_.:]*).*/\1/p' "$src" | head -n 1)
        if [ -z "$module_name" ]; then
            continue
        fi

        local pcm_path="${OUT_DIR}/${module_name}.pcm"
        clang++ "${clang_args[@]}" -x c++-module --precompile "$src" -o "$pcm_path"
        echo "  ${module_name} -> ${pcm_path}"
    done
}

# 交互式配置函数
function interactive_config {
    echo "=== 交互式配置模式 ==="
    
    # 1. 选择操作系统
    echo "请选择目标操作系统:"
    echo "  1) linux (默认)"
    echo "  2) win"
    read -p "输入选项 [1-2]: " os_choice
    case $os_choice in
        2) TARGET_OS="win" ;;
        *) TARGET_OS="linux" ;;
    esac
    echo "已选择系统: $TARGET_OS"
    echo ""

    # 2. 选择架构
    echo "请选择目标架构:"
    echo "  1) x64 (默认)"
    echo "  2) arm64"
    read -p "输入选项 [1-2]: " arch_choice
    case $arch_choice in
        2) TARGET_CPU="arm64" ;;
        *) TARGET_CPU="x64" ;;
    esac
    echo "已选择架构: $TARGET_CPU"
    echo ""

    # 3. 选择构建类型
    echo "请选择构建类型:"
    echo "  1) Debug (默认)"
    echo "  2) Release"
    read -p "输入选项 [1-2]: " type_choice
    case $type_choice in
        2) BUILD_TYPE="Release" ;;
        *) BUILD_TYPE="Debug" ;;
    esac
    echo "已选择类型: $BUILD_TYPE"
    echo ""
    
    # 4. 是否清理
    read -p "是否在构建前清理输出目录? [y/N]: " clean_choice
    if [[ "$clean_choice" =~ ^[Yy]$ ]]; then
        CLEAN_BUILD=true
        echo "已选择: 清理构建"
    else
        CLEAN_BUILD=false
    fi

    # 5. 选择编译器
    echo ""
    echo "请选择编译器:"
    echo "  1) gcc (默认)"
    echo "  2) clang"
    read -p "输入选项 [1-2]: " cc_choice
    case $cc_choice in
        2) COMPILER="clang" ;;
        *) COMPILER="gcc" ;;
    esac
    echo "已选择编译器: $COMPILER"
    echo "======================"
}

# 解析参数
CLEAN_BUILD=false
MOVE_ROOT=false
EXPORT_ASM=false
ENABLE_GC_SECTIONS=false

while [[ "$#" -gt 0 ]]; do
    case $1 in
        -i|--interactive) interactive_config ;;
        --debug) BUILD_TYPE="Debug" ;;
        --release) BUILD_TYPE="Release" ;;
        --arch) TARGET_CPU="$2"; shift ;;
        --os) TARGET_OS="$2"; shift ;;
        --compiler) COMPILER="$2"; shift ;;
        --target) BUILD_TARGET="$2"; shift ;;
        --clean) CLEAN_BUILD=true ;;
        --gc-sections) ENABLE_GC_SECTIONS=true ;;
        --move-root) MOVE_ROOT=true ;;
        --asm) EXPORT_ASM=true ;;
        --help) show_help; exit 0 ;;
        *) echo "未知参数: $1"; show_help; exit 1 ;;
    esac
    shift
done

# 参数校验
if [[ "$COMPILER" != "gcc" && "$COMPILER" != "clang" ]]; then
    echo "不支持的编译器: $COMPILER (仅支持: gcc, clang)"
    exit 1
fi


# 定义输出目录 (例如: out/linux-x64-Debug-clang)
OUT_DIR="${ROOT_DIR}/out/${TARGET_OS}-${TARGET_CPU}-${BUILD_TYPE}-${COMPILER}"

# 如果需要清理
if [ "$CLEAN_BUILD" = true ]; then
    echo "正在清理构建目录: ${OUT_DIR}..."
    rm -rf "${OUT_DIR}"
fi

# 组装 GN 参数
GN_ARGS="target_os=\"${TARGET_OS}\" target_cpu=\"${TARGET_CPU}\""

if [ "$BUILD_TYPE" == "Debug" ]; then
    GN_ARGS="${GN_ARGS} is_debug=true"
else
    GN_ARGS="${GN_ARGS} is_debug=false"
fi

if [ "$COMPILER" == "clang" ]; then
    GN_ARGS="${GN_ARGS} is_clang=true"
else
    GN_ARGS="${GN_ARGS} is_clang=false"
fi

if [ -n "${BUILD_TARGET}" ]; then
    GN_ARGS="${GN_ARGS} build_target=\"${BUILD_TARGET}\""
fi

if [ "${ENABLE_GC_SECTIONS}" = true ]; then
    GN_ARGS="${GN_ARGS} enable_gc_sections=true"
fi

echo "========================================"
echo "构建配置:"
echo "  类型: ${BUILD_TYPE}"
echo "  系统: ${TARGET_OS}"
echo "  架构: ${TARGET_CPU}"
echo "  编译器: ${COMPILER}"
echo "  目标: ${BUILD_TARGET}"
echo "  去除未引用函数: ${ENABLE_GC_SECTIONS}"
echo "  输出: ${OUT_DIR}"
echo "  参数: ${GN_ARGS}"
echo "========================================"

# 1. 生成构建文件
echo "正在生成构建文件 (gn gen)..."
# Always write args.gn
"${ROOT_DIR}/build/generate_args_gn.sh" "${OUT_DIR}" --target "${BUILD_TARGET}" --os "${TARGET_OS}" --arch "${TARGET_CPU}" --compiler "${COMPILER}" $( [ "${BUILD_TYPE}" == "Debug" ] && echo --debug || echo --release ) $( [ "${ENABLE_GC_SECTIONS}" = true ] && echo --gc-sections )

if [ "$MOVE_ROOT" = true ]; then
    # Try to copy the original/backup root BUILD.gn into out and run gn with out as root
    if [ -f "${ROOT_DIR}/BUILD.gn" ]; then
        SRC_BUILD_GN="${ROOT_DIR}/BUILD.gn"
    elif [ -f "${ROOT_DIR}/BUILD.gn.bak" ]; then
        SRC_BUILD_GN="${ROOT_DIR}/BUILD.gn.bak"
    else
        echo "No suitable root BUILD.gn backup found; falling back to generated fragment"
        SRC_BUILD_GN=""
    fi

    if [ -n "$SRC_BUILD_GN" ]; then
        mkdir -p "${OUT_DIR}"
        # Create an out-root BUILD.gn based on the repository BUILD.gn but
        # pointing the generated import to the local generated_build.gni file.
        mkdir -p "${OUT_DIR}"
        sed 's#import(.*generated_build.gni.*)#import("generated_build.gni")#' "$SRC_BUILD_GN" > "${OUT_DIR}/BUILD.gn" || cp "$SRC_BUILD_GN" "${OUT_DIR}/BUILD.gn"
        # Copy .gn and build/ if present so GN still finds buildconfig and other includes
        if [ -f "${ROOT_DIR}/.gn" ]; then
            cp "${ROOT_DIR}/.gn" "${OUT_DIR}/.gn"
        fi
        if [ -d "${ROOT_DIR}/build" ]; then
            cp -a "${ROOT_DIR}/build" "${OUT_DIR}/build"
        fi
        # Copy source subdirectories that contain BUILD.gn so GN can resolve //labels
        for d in test benchmark libuniopt; do
            if [ -d "${ROOT_DIR}/$d" ]; then
                cp -a "${ROOT_DIR}/$d" "${OUT_DIR}/$d"
            fi
        done
        # Also generate the generated_build.gni fragment in out so the copied BUILD.gn's
        # import("generated_build.gni") will succeed and include generated_default_deps.
        "${ROOT_DIR}/build/generate_root_BUILD_gn.sh" "${OUT_DIR}"
        echo "Copied root BUILD.gn and supporting files to ${OUT_DIR}; running gn with out as root"
        gn gen "${OUT_DIR}" --root="${OUT_DIR}"
    else
        # fallback to generating fragment into out
        "${ROOT_DIR}/build/generate_root_BUILD_gn.sh" "${OUT_DIR}"
        gn gen "${OUT_DIR}"
    fi
else
    if [ -f "${ROOT_DIR}/BUILD.gn" ]; then
        # Repo provides a root BUILD.gn. Still generate the out fragment so
        # per-target builds (via generated_default_deps) work and out/generated_build.gni
        # exists for the imported file.
        "${ROOT_DIR}/build/generate_root_BUILD_gn.sh" "${OUT_DIR}"
        # Use repository root when generating build files
        gn gen "${OUT_DIR}"
    else
        # No repo BUILD.gn — create minimal out/BUILD.gn that imports generated fragment,
        # copy small set of support files and run gn with out as root.
        "${ROOT_DIR}/build/generate_root_BUILD_gn.sh" "${OUT_DIR}"

        cat > "${OUT_DIR}/BUILD.gn" <<'GN'
declare_args() {
  build_target = ""
}

import("generated_build.gni")

group("default") {
  deps = []
  if (defined(generated_default_deps)) {
    deps += generated_default_deps
  }
}
GN

        if [ -f "${ROOT_DIR}/.gn" ]; then
            cp "${ROOT_DIR}/.gn" "${OUT_DIR}/.gn"
        fi
        if [ -d "${ROOT_DIR}/build" ]; then
            cp -a "${ROOT_DIR}/build" "${OUT_DIR}/build"
        fi
        for d in test benchmark libuniopt; do
            if [ -d "${ROOT_DIR}/$d" ]; then
                cp -a "${ROOT_DIR}/$d" "${OUT_DIR}/$d"
            fi
        done

        gn gen "${OUT_DIR}" --root="${OUT_DIR}"
    fi
fi

precompile_clang_modules

# 生成 IntelliSense 专用编译数据库（去掉命令中的后处理片段）
# 仅在 intellisense 文件存在时复制，避免在某些 GN 生成配置下失败
if [ -f "${OUT_DIR}/compile_commands.intellisense.json" ]; then
    cp "${OUT_DIR}/compile_commands.intellisense.json" "${ROOT_DIR}/compile_commands.json"
fi

# 2. 编译
# Determine specific ninja target based on build target naming conventions:
# - If BUILD_TARGET contains '/' or ':' assume it's a full ninja label and use it directly.
# - If BUILD_TARGET matches "<dir>_<name>", map to "//<dir>:<name>" (e.g. test_cpy -> //test:test_cpy).
# - If BUILD_TARGET matches "<dir>_all" map to "//<dir>:all" (e.g. test_all -> //test:all).
# - Otherwise fall back to default top-level build (no specific target).
NINJA_TARGET=""
if [[ "${BUILD_TARGET}" == *"/"* || "${BUILD_TARGET}" == *":"* ]]; then
    NINJA_TARGET="${BUILD_TARGET}"
elif find_target_label; then
    :
elif [[ "${BUILD_TARGET}" == *_* ]]; then
    DIR="${BUILD_TARGET%%_*}"
    # Use full BUILD_TARGET as target name (e.g. test_cpy -> //test:test_cpy)
    if [[ "${BUILD_TARGET}" == "${DIR}_all" ]]; then
        NINJA_TARGET="//${DIR}:all"
    else
        NINJA_TARGET="//${DIR}:${BUILD_TARGET}"
    fi
fi

echo "正在编译 (ninja)..."
if [[ -n "${NINJA_TARGET}" ]]; then
    echo "Building ninja target: ${NINJA_TARGET}"
    # Ninja expects target names without leading // when invoked via -C
    NINJA_TO_USE="${NINJA_TARGET#//}"
    ninja -C "${OUT_DIR}" -j1 "${NINJA_TO_USE}" 2>&1 | tee "${OUT_DIR}/build.log"
else
    ninja -C "${OUT_DIR}" -j1 2>&1 | tee "${OUT_DIR}/build.log"
fi

if [ "${EXPORT_ASM}" = true ]; then
    export_asm_artifacts "${OUT_DIR}" "${NINJA_TARGET}"
fi

echo "构建成功! 构建产物位于: ${OUT_DIR}"
echo "构建日志已保存: ${OUT_DIR}/build.log"
