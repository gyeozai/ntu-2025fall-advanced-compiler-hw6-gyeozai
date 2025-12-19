#!/bin/bash

# Step 1: Define paths and locate LLVM tools (Clang/Opt)
PASS_LIB="../src/build/libSimpleLoopLICM.so"
if [ ! -f "$PASS_LIB" ]; then PASS_LIB="../src/build/SimpleLoopLICM.so"; fi

if command -v clang-17 &> /dev/null; then CLANG=clang-17; OPT=opt-17; else CLANG=clang; OPT=opt; fi

TESTS=("test1_simple" "test2_nested" "test3_complex" "test4_dependency" "test5_safety")

# Step 2: Initialize color codes and data arrays for benchmarking
GREEN='\033[0;32m'
RED='\033[0;31m'
NC='\033[0m' # No Color

declare -A compile_orig_arr compile_opt_arr
declare -A size_orig_arr size_opt_arr
declare -A time_orig_arr time_opt_arr
declare -A mem_orig_arr mem_opt_arr
declare -A status_arr
declare -A output_orig_arr output_opt_arr

RUN_COUNT=5

# Step 3: Define a helper function to calculate the minimum execution time over multiple runs
get_min_time() {
    local exe=$1
    local out_file=$2
    local min_t=9999.0
    local time_tmp="results/time_temp.txt"
    
    for i in $(seq 1 $RUN_COUNT); do
        /usr/bin/time -f "%e" -o "$time_tmp" $exe > "$out_file" 2>/dev/null
        
        if [ -f "$time_tmp" ]; then
            val=$(cat "$time_tmp")
            if [[ "$val" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
                 min_t=$(awk "BEGIN {if ($val < $min_t) print $val; else print $min_t}")
            fi
        fi
    done
    rm -f "$time_tmp"
    echo $min_t
}

echo ""
echo "==========================================================="
echo "                     Correctness Check                     "
echo "==========================================================="

pass_count=0
total_tests=${#TESTS[@]}

# Step 4: Data Collection Phase
# Iterate through each test case to measure compilation, size, time, and memory
for test in "${TESTS[@]}"; do
    # 4.1: Measure Compile Time (Original vs Optimized pipeline)
    start_time=$(date +%s%N)
    $OPT -passes='mem2reg,instcombine,loop-simplify,loop-rotate' \
        -disable-output results/${test}.ll > /dev/null 2>&1
    end_time=$(date +%s%N)
    compile_orig_arr[$test]=$(( (end_time - start_time) / 1000000 ))

    start_time=$(date +%s%N)
    $OPT -load-pass-plugin=$PASS_LIB \
        -passes='mem2reg,instcombine,loop-simplify,loop-rotate,simple-licm,dce' \
        -disable-output results/${test}.ll > /dev/null 2>&1
    end_time=$(date +%s%N)
    compile_opt_arr[$test]=$(( (end_time - start_time) / 1000000 ))

    # 4.2: Record Binary Code Size
    if [ -f "results/${test}.o" ]; then
        size_orig_arr[$test]=$(wc -c < results/${test}.o | tr -d ' ')
    else
        size_orig_arr[$test]=0
    fi
    
    if [ -f "results/${test}_opt.o" ]; then
        size_opt_arr[$test]=$(wc -c < results/${test}_opt.o | tr -d ' ')
    else
        size_opt_arr[$test]=0
    fi

    # 4.3: Measure Execution Time (taking the minimum of 5 runs)
    time_orig_arr[$test]=$(get_min_time "./results/${test}_original" "results/output_${test}_orig.txt")
    time_opt_arr[$test]=$(get_min_time "./results/${test}_optimized" "results/output_${test}_opt.txt")

    # 4.4: Track Peak Memory Usage
    mem_orig_arr[$test]=$( /usr/bin/time -f "%M" ./results/${test}_original 2>&1 >/dev/null )
    mem_opt_arr[$test]=$( /usr/bin/time -f "%M" ./results/${test}_optimized 2>&1 >/dev/null )

    # 4.5: Perform Correctness Verification
    # Compare raw output of original vs optimized binaries
    output_orig=$(cat results/output_${test}_orig.txt 2>/dev/null | tr -d '\n')
    output_opt=$(cat results/output_${test}_opt.txt 2>/dev/null | tr -d '\n')
    
    output_orig_arr[$test]=$output_orig
    output_opt_arr[$test]=$output_opt
    
    if diff -w -q results/output_${test}_orig.txt results/output_${test}_opt.txt >/dev/null 2>&1; then
        status_arr[$test]="PASS"
        ((pass_count++))
        status_display="${GREEN}PASS${NC}"
    else
        status_arr[$test]="FAIL"
        status_display="${RED}FAIL${NC}"
    fi
    
    echo "Test Case: ${test}"
    echo "  Original output:  ${output_orig}"
    echo "  Optimized output: ${output_opt}"
    echo -e "  Result: ${status_display}"
    echo "-----------------------------------------------------------"
done

echo "* Testcases passed: ${pass_count}/${total_tests}"
echo "==========================================================="
echo ""

# Step 5: Report Generation Phase
# Format the collected data into a performance comparison table
echo "================================================================================"
echo "                   Performance Benchmark: Loop LICM                            "
echo "================================================================================"
printf "%-16s | %-14s | %12s | %12s | %12s\n" "Test Case" "Metric" "Original" "Optimized" "Diff"
echo "--------------------------------------------------------------------------------"

for test in "${TESTS[@]}"; do
    compile_orig=${compile_orig_arr[$test]}
    compile_opt=${compile_opt_arr[$test]}
    size_orig=${size_orig_arr[$test]}
    size_opt=${size_opt_arr[$test]}
    min_time_orig=${time_orig_arr[$test]}
    min_time_opt=${time_opt_arr[$test]}
    mem_orig=${mem_orig_arr[$test]}
    mem_opt=${mem_opt_arr[$test]}
    
    time_orig_ms=$(awk "BEGIN {printf \"%.0f\", $min_time_orig * 1000}")
    time_opt_ms=$(awk "BEGIN {printf \"%.0f\", $min_time_opt * 1000}")
    
    if [ "$min_time_orig" = "9999.0" ] || [ "$min_time_opt" = "9999.0" ]; then
        diff_time="ERR"
    else
        diff_time=$(awk "BEGIN {printf \"%+.0f\", ($min_time_opt - $min_time_orig) * 1000}")
    fi
    
    printf "%-16s | %-14s | %9d ms | %9d ms | %+9d ms\n" "$test" "Compile Time" "$compile_orig" "$compile_opt" "$((compile_opt - compile_orig))"
    printf "%-16s | %-14s | %9d B  | %9d B  | %+9d B\n"  ""      "Code Size"    "$size_orig"   "$size_opt"    "$((size_opt - size_orig))"
    printf "%-16s | %-14s | %9s ms | %9s ms | %9s ms\n"  ""      "Exec Time(min)"   "$time_orig_ms"     "$time_opt_ms"      "$diff_time"
    printf "%-16s | %-14s | %9d KB | %9d KB | %+9d KB\n" ""      "Memory Usage" "$mem_orig"    "$mem_opt"     "$((mem_opt - mem_orig))"
    echo "--------------------------------------------------------------------------------"
done

echo "* Exec Time is the MINIMUM of $RUN_COUNT runs."
echo "================================================================================"

echo ""
echo "All tests completed!"