#!/bin/bash
set -e
cd "$(dirname "$0")/.."

rm -rf build-linux
./scripts/build.sh
