#!/bin/bash

# Script to analyze code size results

echo "Code Size Analysis"
echo "=================="
echo ""

TESTS=("test1_simple" "test2_nested" "test3_complex" "test4_dependency" "test5_safety")

# Function to count instructions in LLVM IR
count_ir_instructions() {
    grep -c "^\s*%" "$1" || echo "0"
}

# Function to count lines in assembly
count_asm_lines() {
    grep -c "^\s*[a-z]" "$1" || echo "0"
}

# Function to get file size
get_size() {
    stat -f%z "$1" 2>/dev/null || stat -c%s "$1" 2>/dev/null || echo "0"
}

echo "LLVM IR Instruction Count:"
echo "--------------------------"
printf "%-20s %10s %10s %10s\n" "Test" "Original" "Optimized" "Difference"
echo "--------------------------------------------------------"

for test in "${TESTS[@]}"; do
    orig=$(count_ir_instructions "results/${test}.ll")
    opt=$(count_ir_instructions "results/${test}_opt.ll")
    diff=$((opt - orig))
    
    printf "%-20s %10d %10d %+10d\n" "$test" "$orig" "$opt" "$diff"
done

echo ""
echo "Assembly Line Count:"
echo "--------------------"
printf "%-20s %10s %10s %10s\n" "Test" "Original" "Optimized" "Difference"
echo "--------------------------------------------------------"

for test in "${TESTS[@]}"; do
    orig=$(count_asm_lines "results/${test}.s")
    opt=$(count_asm_lines "results/${test}_opt.s")
    diff=$((opt - orig))
    
    printf "%-20s %10d %10d %+10d\n" "$test" "$orig" "$opt" "$diff"
done

echo ""
echo "Object File Size (bytes):"
echo "-------------------------"
printf "%-20s %10s %10s %10s\n" "Test" "Original" "Optimized" "Difference"
echo "--------------------------------------------------------"

for test in "${TESTS[@]}"; do
    orig=$(get_size "results/${test}.o")
    opt=$(get_size "results/${test}_opt.o")
    diff=$((opt - orig))
    
    printf "%-20s %10d %10d %+10d\n" "$test" "$orig" "$opt" "$diff"
done

echo ""
echo "Executable Size (bytes):"
echo "------------------------"
printf "%-20s %10s %10s %10s\n" "Test" "Original" "Optimized" "Difference"
echo "--------------------------------------------------------"

for test in "${TESTS[@]}"; do
    orig=$(get_size "results/${test}_original")
    opt=$(get_size "results/${test}_optimized")
    diff=$((opt - orig))
    
    printf "%-20s %10d %10d %+10d\n" "$test" "$orig" "$opt" "$diff"
done

echo ""
echo "Verification (running executables):"
echo "------------------------------------"

for test in "${TESTS[@]}"; do
    echo "Testing $test:"
    echo -n "  Original output: "
    results/${test}_original
    echo -n "  Optimized output: "
    results/${test}_optimized
    echo ""
done

echo "Analysis complete!"