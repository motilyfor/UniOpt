#!/usr/bin/env bash
set -euo pipefail

# Usage: ./test_linux_clang.sh <target> [-- args-for-binary]
# Example: ./test_linux_clang.sh test_opt

if [ "$#" -eq 0 ]; then
  TARGET=test_opt
  ARGS=()
elif [ "$#" -eq 1 ]; then
  # single argument: treat as test name, keep default target
  TARGET=test_opt
  ARGS=("$1")
else
  # two or more args: first is target, rest are args for the binary
  TARGET=$1
  shift
  ARGS=("$@")
fi

ROOT_DIR=$(cd "$(dirname "$0")" && pwd)

"$ROOT_DIR/build_linux_clang.sh" --target "$TARGET"

OUT_DIR="$ROOT_DIR/out/linux-x64-Debug-clang"
BIN="$OUT_DIR/$TARGET"

if [[ -x "$BIN" ]]; then
  echo "Running $BIN..."
  "$BIN" "${ARGS[@]}"
  exit $?
else
  echo "Build succeeded but binary not found: $BIN" >&2
  ls -la "$OUT_DIR" || true
  exit 2
fi
