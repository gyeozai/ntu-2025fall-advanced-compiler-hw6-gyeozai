#!/bin/bash

# Build script for Simple Loop LICM Pass

set -e

echo "Building Simple Loop LICM Pass..."

# Create build directory
mkdir -p build
cd build

# Configure with CMake
cmake ..

# Build
make

echo "Build complete! The pass is located at: build/SimpleLoopLICM.so"
echo ""
echo "To use the pass:"
echo "  clang -O0 -emit-llvm -S test.c -o test.ll"
echo "  opt -load-pass-plugin=./build/SimpleLoopLICM.so -passes=simple-licm -S test.ll -o test_opt.ll"