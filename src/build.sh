#!/bin/bash

# Build script for Simple Loop LICM Pass

set -e

echo "Building Simple Loop LICM Pass..."

# Step 1: Create a build directory to keep the source tree clean
mkdir -p build
cd build

# Step 2: Configure the project with CMake 
# This detects LLVM and generates the necessary Makefiles
cmake ..

# Step 3: Compile the project
# This produces the shared object (.so) file
make

echo "Build complete! The pass is located at: build/SimpleLoopLICM.so"
echo ""

# Step 4: Output usage instructions for the user
echo "To use the pass:"
echo "  1. Compile C code to LLVM IR: clang -O0 -emit-llvm -S test.c -o test.ll"
echo "  2. Run the pass using opt: opt -load-pass-plugin=./build/SimpleLoopLICM.so -passes=simple-licm -S test.ll -o test_opt.ll"