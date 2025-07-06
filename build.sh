#!/usr/bin/bash

set -e

cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build
./build/alltests
valgrind --leak-check=full --error-exitcode=42 ./build/alltests
lcov -c --demangle-cpp -d . -o coverage.info --branch-coverage  --ignore-errors mismatch --include src --exclude test-src
genhtml -branch-coverage coverage.info --output-directory out
