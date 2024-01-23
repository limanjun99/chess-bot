#!/bin/sh

set -e

parent_path=$(dirname "$0")/../
cd $parent_path

echo "Running chess library tests...\n"
./build/test/chess/chess_tests

echo "\n\nRunning chess engine tests...\n"
./build/test/engine/chess_engine_tests