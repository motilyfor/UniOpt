#!/bin/bash

set -e

./build.sh --debug --arch x64 --os linux --compiler clang --clean "$@"