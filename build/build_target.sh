#!/bin/bash

set -euo pipefail

# Wrapper in build/ to forward build options to top-level build.sh
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ROOT_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"

if [ "$#" -eq 0 ]; then
    echo "Usage: $0 [build.sh options]
Example: $0 --target test_cpy --compiler clang --clean"
    exit 1
fi

exec "${ROOT_DIR}/build.sh" "$@"
