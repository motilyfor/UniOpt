#!/bin/bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

if [ "$#" -lt 2 ]; then
    echo "Usage: $0 <out_dir> --target <target> [--os <os>] [--arch <arch>] [--debug|--release] [--compiler <gcc|clang>] [--gc-sections]"
    exit 1
fi

# default args
TARGET_OS="linux"
TARGET_CPU="x64"
BUILD_TYPE="Debug"
COMPILER="gcc"
BUILD_TARGET=""
ENABLE_GC_SECTIONS="false"

while [[ "$#" -gt 0 ]]; do
    case "$1" in
        --target) BUILD_TARGET="$2"; shift 2 ;;
        --os) TARGET_OS="$2"; shift 2 ;;
        --arch) TARGET_CPU="$2"; shift 2 ;;
        --compiler) COMPILER="$2"; shift 2 ;;
        --debug) BUILD_TYPE="Debug"; shift ;;
        --release) BUILD_TYPE="Release"; shift ;;
        --gc-sections) ENABLE_GC_SECTIONS="true"; shift ;;
        *) OUT_DIR="$1"; shift ;;
    esac
done

if [ -z "${OUT_DIR:-}" ]; then
    echo "missing out_dir"
    exit 1
fi

mkdir -p "${OUT_DIR}"

# find target label helper (returns 0 and outputs label)
find_target_label() {
    local name="$1"
    if [[ -z "$name" ]]; then
        return 1
    fi
    if [[ "$name" == *"/"* || "$name" == *":"* ]]; then
        echo "$name"
        return 0
    fi

    local -a matches
    mapfile -t matches < <(grep -Rsl --include=BUILD.gn -E "^(\s)*(executable|static_library|shared_library|source_set|group)\\(\\s*\"${name}\"\\s*\\)" "$ROOT_DIR" --exclude-dir=.git --exclude-dir=out --exclude-dir=.vscode || true)
    if [ "${#matches[@]}" -eq 0 ]; then
        mapfile -t matches < <(grep -Rsl --include=BUILD.gn -F "\"${name}\"" "$ROOT_DIR" --exclude-dir=.git --exclude-dir=out --exclude-dir=.vscode || true)
    fi
    if [ "${#matches[@]}" -eq 0 ]; then
        return 1
    fi
    local chosen=${matches[0]}
    local rel=${chosen#${ROOT_DIR}/}
    local dir=$(dirname "$rel")
    local label
    if [ "$dir" = "." ] || [ -z "$dir" ]; then
        label="//:${name}"
    else
        label="//${dir}:${name}"
    fi
    echo "$label"
    return 0
}

if [ -n "$BUILD_TARGET" ]; then
    FULL_LABEL="$(find_target_label "$BUILD_TARGET")" || {
        echo "Error: cannot resolve target '$BUILD_TARGET'"
        exit 1
    }
else
    FULL_LABEL=""
fi

# produce args.gn content
ARGS_FILE="${OUT_DIR}/args.gn"
{
    echo "target_os=\"${TARGET_OS}\""
    echo "target_cpu=\"${TARGET_CPU}\""
    if [ "$BUILD_TYPE" = "Debug" ]; then
        echo "is_debug=true"
    else
        echo "is_debug=false"
    fi
    if [ "$COMPILER" = "clang" ]; then
        echo "is_clang=true"
    else
        echo "is_clang=false"
    fi
    if [ -n "$FULL_LABEL" ]; then
        # write as quoted string; generator will parse and embed label into BUILD.gn
        echo "build_target=\"${FULL_LABEL}\""
    fi
    echo "enable_gc_sections=${ENABLE_GC_SECTIONS}"
} > "$ARGS_FILE"

echo "Wrote GN args to: ${ARGS_FILE}"

exit 0

