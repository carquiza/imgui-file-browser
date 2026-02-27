#!/bin/bash
set -e
cd "$(dirname "$0")/.."

# Find vcpkg toolchain
TOOLCHAIN_ARG=""
if [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
    TOOLCHAIN_ARG="-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
elif [ -f "$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    TOOLCHAIN_ARG="-DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"
fi

mkdir -p build-linux
cmake -B build-linux -DCMAKE_BUILD_TYPE=Release $TOOLCHAIN_ARG
cmake --build build-linux -j$(nproc)
