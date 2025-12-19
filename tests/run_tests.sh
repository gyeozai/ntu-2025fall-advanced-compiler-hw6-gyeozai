#!/bin/bash

# Step 1: Environment Setup
# Detect and set paths for LLVM tools (Clang, Opt, LLC)
set -e

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

# Step 2: Validate Pass Plugin
# Ensure the custom LLVM pass library exists before proceeding
PASS_LIB="../src/build/libSimpleLoopLICM.so"

if [ ! -f "$PASS_LIB" ]; then
    PASS_LIB="../src/build/SimpleLoopLICM.so"
fi

if [ ! -f "$PASS_LIB" ]; then
    echo "Error: Pass library not found"
    echo "Please build the pass first by running: cd ../src && ./build.sh"
    exit 1
fi

echo "Using pass library: $PASS_LIB"

# Step 3: Initialize Output Directory
mkdir -p results
TESTS=("test1_simple" "test2_nested" "test3_complex" "test4_dependency" "test5_safety")

# Step 4: Iterative Compilation and Optimization
for test in "${TESTS[@]}"; do
    echo ""
    echo "Processing $test.c..."
    
    # 4.1: Source to LLVM IR
    # We use -O1 but disable passes to get a clean IR starting point for our pass
    $CLANG -O1 -Xclang -disable-llvm-passes -emit-llvm -S ${test}.c -o results/${test}.ll
    
    # 4.2: Apply Custom Optimization Pipeline
    # Pipeline: Convert to SSA (mem2reg) -> Prepare loops (rotate) -> Run CUSTOM LICM -> Clean up (dce)
    echo "  Running optimization pass..."
    
    $OPT -load-pass-plugin=$PASS_LIB \
        -passes='mem2reg,instcombine,loop-simplify,loop-rotate,simple-licm,dce' -S \
        results/${test}.ll -o results/${test}_opt.ll 2>&1 | grep -E "(Processing|Found|Checking|Successfully|Hoisting)"
    
    # 4.3: Lowering (IR to Assembly)
    # Convert both original and optimized IR into target-specific assembly code
    $LLC results/${test}.ll -o results/${test}.s
    $LLC results/${test}_opt.ll -o results/${test}_opt.s

    # 4.4: Assembler (Assembly to Object Files)
    # Explicitly generate .o files for static size analysis
    $CLANG -c results/${test}.s -o results/${test}.o
    $CLANG -c results/${test}_opt.s -o results/${test}_opt.o
    
    # 4.5: Linking (Object to Executables)
    # Generate the final binaries. -no-pie is used to ensure compatibility in certain environments
    $CLANG -no-pie results/${test}.s -o results/${test}_original
    $CLANG -no-pie results/${test}_opt.s -o results/${test}_optimized
    
    echo "  ✓ Generated executables"
done

echo ""
echo "All tests completed!"