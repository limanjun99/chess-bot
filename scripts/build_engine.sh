#!/bin/bash

# The purpose of this script is to build the engine_cli binary for the current commit,
# and output it to 'engine_builds/engine_cli_[$1]'.
#
# This script is necessary as build instructions may change over versions.

# Exit on error.
set -e

# Go to root directory.
scriptdir="$(dirname "$0")"
cd $scriptdir
cd ../

# Build.
rm -rf build/
cmake --preset=release
cmake --build --preset=release --target=engine_cli

# Copy binary.
mkdir -p engine_builds/
cp build/release/bin/engine_cli engine_builds/engine_cli_$1

# Clear build directory.
rm -rf build/