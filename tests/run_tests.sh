#!/bin/bash

# Script to run all tests and generate optimized versions

set -e

# Try to use LLVM 17 tools explicitly
if command -v clang-17 &> /dev/null; then
    CLANG=clang-17
    OPT=opt-17
    LLC=llc-17
elif command -v clang &> /dev/null; then
    CLANG=clang
    OPT=opt
    LLC=llc
else
    echo "Error: clang not found"
    exit 1
fi

echo "Using: $CLANG, $OPT, $LLC"

PASS_LIB="../src/build/libSimpleLoopLICM.so"

# Try alternative name if not found
if [ ! -f "$PASS_LIB" ]; then
    PASS_LIB="../src/build/SimpleLoopLICM.so"
fi

if [ ! -f "$PASS_LIB" ]; then
    echo "Error: Pass library not found"
    echo "Please build the pass first by running: cd ../src && ./build.sh"
    exit 1
fi

echo "Using pass library: $PASS_LIB"

echo "Running tests..."
echo "=================="

# Create output directory
mkdir -p results

# Test files
TESTS=("test1_simple" "test2_nested" "test3_complex" "test4_dependency" "test5_safety")

for test in "${TESTS[@]}"; do
    echo ""
    echo "Processing $test.c..."
    
    # 1. Compile to LLVM IR
    # 使用 -O1 但加上 -disable-llvm-passes 獲得比較乾淨的 IR
    $CLANG -O1 -Xclang -disable-llvm-passes -emit-llvm -S ${test}.c -o results/${test}.ll
    
    # 2. Apply our optimization pass
    # 順序: mem2reg (轉 SSA) -> simplify/rotate (整理迴圈) -> OUR_PASS -> instcombine/dce (清理)
    echo "  Running optimization pass..."
    
    $OPT -load-pass-plugin=$PASS_LIB \
        -passes='mem2reg,instcombine,loop-simplify,loop-rotate,simple-licm,dce' -S \
        results/${test}.ll -o results/${test}_opt.ll 2>&1 | grep -E "(Processing|Found|Checking|Successfully|Hoisting)"
    
    # 3. Generate Assembly, Object Files & Executables
    # Convert IR to Assembly
    $LLC results/${test}.ll -o results/${test}.s
    $LLC results/${test}_opt.ll -o results/${test}_opt.s

    # [FIX] Explicitly generate Object Files for Code Size analysis
    $CLANG -c results/${test}.s -o results/${test}.o
    $CLANG -c results/${test}_opt.s -o results/${test}_opt.o
    
    # Link to Executables (using -no-pie to fix linker error)
    $CLANG -no-pie results/${test}.s -o results/${test}_original
    $CLANG -no-pie results/${test}_opt.s -o results/${test}_optimized
    
    echo "  ✓ Generated executables"
done

echo ""
echo "All tests completed!"