#!/usr/bin/env bash

set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

# defaults
BUILD_TYPE="Debug"
TARGET_OS="linux"
TARGET_CPU="x64"
COMPILER="gcc"
CLEAN_BUILD=false
BUILD_TARGET=""

function show_help {
    cat <<EOF
Usage: $0 [options]
Options:
  --debug|--release    build type
  --os <os>            target OS
  --arch <arch>        target cpu
  --compiler <cc>      gcc|clang
  --target <name>      short name or full label
  --clean              remove out dir before generating
  --help               show this help
EOF
}

while [[ "$#" -gt 0 ]]; do
    case "$1" in
        --debug) BUILD_TYPE="Debug"; shift ;;
        --release) BUILD_TYPE="Release"; shift ;;
        --os) TARGET_OS="$2"; shift 2 ;;
        --arch) TARGET_CPU="$2"; shift 2 ;;
        --compiler) COMPILER="$2"; shift 2 ;;
        --target) BUILD_TARGET="$2"; shift 2 ;;
        --clean) CLEAN_BUILD=true; shift ;;
        --help) show_help; exit 0 ;;
        *) echo "Unknown arg: $1"; show_help; exit 1 ;;
    esac
done

OUT_DIR="${ROOT_DIR}/out/${TARGET_OS}-${TARGET_CPU}-${BUILD_TYPE}-${COMPILER}"

if [ "$CLEAN_BUILD" = true ]; then
    echo "Cleaning ${OUT_DIR}..."
    rm -rf "${OUT_DIR}"
fi

mkdir -p "${OUT_DIR}"
"${ROOT_DIR}/build/generate_args_gn.sh" "${OUT_DIR}" --target "${BUILD_TARGET}" --os "${TARGET_OS}" --arch "${TARGET_CPU}" --compiler "${COMPILER}" $( [ "$BUILD_TYPE" = "Debug" ] && echo --debug || echo --release )

# Use helper to write args.gn into OUT_DIR and generate top-level BUILD.gn
"${ROOT_DIR}/build/generate_root_BUILD_gn.sh" "${OUT_DIR}"
# delegate to helper to write args.gn
"${ROOT_DIR}/build/generate_args_gn.sh" "${OUT_DIR}" --target "${BUILD_TARGET}" --os "${TARGET_OS}" --arch "${TARGET_CPU}" --compiler "${COMPILER}" $( [ "$BUILD_TYPE" = "Debug" ] && echo --debug || echo --release )

echo "Generating GN build files..."
gn gen "${OUT_DIR}"

echo "Precompilation / additional steps skipped in local builder."

# compute ninja label if short name provided
NINJA_TARGET=""
if [[ -n "$BUILD_TARGET" ]]; then
    if [[ "$BUILD_TARGET" == *"/"* || "$BUILD_TARGET" == *":"* ]]; then
        NINJA_TARGET="$BUILD_TARGET"
    else
        DIR="${BUILD_TARGET%%_*}"
        NAME="${BUILD_TARGET#${DIR}_}"
        if [[ "$NAME" == "all" ]]; then
            NINJA_TARGET="//${DIR}:all"
        else
            # if name contains no underscore, fallback to helper resolution
            if [[ "$DIR" == "$BUILD_TARGET" ]]; then
                # fallback find via grep
                FULL_LABEL=$("${ROOT_DIR}/build/generate_args_gn.sh" "${OUT_DIR}" --target "${BUILD_TARGET}" --os "${TARGET_OS}" --arch "${TARGET_CPU}" --compiler "${COMPILER}" --debug 2>/dev/null || true)
                if [[ -n "$FULL_LABEL" ]]; then
                    NINJA_TARGET="$FULL_LABEL"
                else
                    NINJA_TARGET="//${DIR}:${NAME}"
                fi
            else
                NINJA_TARGET="//${DIR}:${NAME}"
            fi
        fi
    fi
fi

echo "Building..."
if [[ -n "$NINJA_TARGET" ]]; then
    echo "ninja -C ${OUT_DIR} ${NINJA_TARGET}"
    NINJA_TO_USE="${NINJA_TARGET#//}"
    ninja -C "${OUT_DIR}" -j1 "${NINJA_TO_USE}" 2>&1 | tee "${OUT_DIR}/build.log"
else
    ninja -C "${OUT_DIR}" -j1 2>&1 | tee "${OUT_DIR}/build.log"
fi

echo "Build finished. Log: ${OUT_DIR}/build.log"
