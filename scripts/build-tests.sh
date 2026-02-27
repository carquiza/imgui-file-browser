#!/bin/bash
# Build script for ImFileBrowser scaling test (Linux/Raspberry Pi)
# This builds the test application separately from the main library

set -e  # Exit on error
cd "$(dirname "$0")/.."

# Configuration
BUILD_DIR="build-linux-tests"
CONFIG="Debug"

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Find vcpkg toolchain (optional - can use system packages instead)
TOOLCHAIN_ARG=""
if [ -n "$VCPKG_ROOT" ] && [ -f "$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake" ]; then
    TOOLCHAIN_ARG="-DCMAKE_TOOLCHAIN_FILE=$VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake"
    echo -e "${GREEN}Using vcpkg toolchain: $VCPKG_ROOT/scripts/buildsystems/vcpkg.cmake${NC}"
elif [ -f "$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake" ]; then
    TOOLCHAIN_ARG="-DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake"
    echo -e "${GREEN}Using vcpkg toolchain: $HOME/vcpkg/scripts/buildsystems/vcpkg.cmake${NC}"
else
    echo -e "${YELLOW}vcpkg not found - using system packages${NC}"
    echo "Make sure you have installed: libglfw3-dev libgl1-mesa-dev"
fi

# Set overlay ports from environment variable
OVERLAY_ARG=""
if [ -n "$VCPKG_OVERLAY_PORTS" ]; then
    OVERLAY_ARG="-DVCPKG_OVERLAY_PORTS=$VCPKG_OVERLAY_PORTS"
fi

# Share vcpkg_installed globally across all projects
VCPKG_INSTALLED_ARG=""
if [ -n "$DEV_SOURCE_ROOT" ]; then
    VCPKG_INSTALLED_ARG="-DVCPKG_INSTALLED_DIR=$DEV_SOURCE_ROOT/vcpkg_installed"
fi

# Create build directory
mkdir -p "$BUILD_DIR"

# Run CMake
echo ""
echo "=== Configuring with CMake ==="
cd "$BUILD_DIR"

cmake ../test \
    -DCMAKE_BUILD_TYPE=$CONFIG \
    $TOOLCHAIN_ARG \
    $OVERLAY_ARG \
    $VCPKG_INSTALLED_ARG

if [ $? -ne 0 ]; then
    echo ""
    echo -e "${RED}CMake configuration failed!${NC}"
    echo ""
    echo "Make sure you have the required dependencies:"
    echo "  Ubuntu/Debian/Raspberry Pi OS:"
    echo "    sudo apt install libglfw3-dev libgl1-mesa-dev"
    echo ""
    echo "  Or with vcpkg:"
    echo "    vcpkg install glfw3 imgui[glfw-binding,opengl3-binding]"
    echo ""
    cd ..
    exit 1
fi

# Build
echo ""
echo "=== Building ==="
cmake --build . --config $CONFIG -j$(nproc)

if [ $? -ne 0 ]; then
    echo ""
    echo -e "${RED}Build failed!${NC}"
    cd ..
    exit 1
fi

cd ..

echo ""
echo -e "${GREEN}=== Build successful! ===${NC}"
echo ""
echo "Run the test with:"
echo "  ./$BUILD_DIR/bin/ImFileBrowserTest"
echo ""
echo "Controls:"
echo "  CTRL+PLUS  - Increase UI scale"
echo "  CTRL+MINUS - Decrease UI scale"
echo "  CTRL+0     - Reset scale to 1.0"
echo ""

# Optionally run the test
if [ "$1" = "run" ]; then
    echo "Running test..."
    "./$BUILD_DIR/bin/ImFileBrowserTest"
fi
