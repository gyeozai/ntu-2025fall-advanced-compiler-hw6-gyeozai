# Advanced Compiler Course - Homework 6

**Loop Invariant Code Motion (LICM) Optimization Pass in LLVM**

## Introduction

This project implements a Loop Invariant Code Motion (LICM) optimization pass for LLVM 17 as part of the Advanced Compiler Design course (CSIE 5054) at National Taiwan University. The optimization identifies computations within loops that produce identical results across all iterations and relocates them to the loop's preheader, thereby reducing dynamic instruction counts and improving execution performance.

### Key Features

- Significant Performance Gains: Up to 4.5× speedup on arithmetic-intensive loops
- Safety Guarantees: Conservative analysis prevents speculative execution faults
- Comprehensive Testing: Five test cases covering basic functionality, nested loops, dependencies, and edge cases
- Formal Correctness: Rigorous proofs of semantic preservation and safety
- Seamless Integration: Compatible with LLVM's optimization pipeline

## Quick Start

### Prerequisites

- **Operating System**: Ubuntu 24.04 LTS (or compatible Linux distribution)
- **LLVM Toolchain**: LLVM 17 (`clang-17`, `opt-17`, `llc-17`)
- **Build Tools**: CMake 3.13+, GNU Make
- **Utilities**: GNU time

### Installation

```bash
# Clone the repository
git clone https://github.com/gyeozai/ntu-2025fall-advanced-compiler-hw6-gyeozai.git
cd ntu-2025fall-advanced-compiler-hw6-gyeozai

# Build the LLVM pass
cd src
rm -rf build
./build.sh

# Run tests and benchmarks
cd ../tests
rm -rf results
./run_tests.sh
./analyze_results_advanced.sh
```

## Project Structure

```
.
├── README.md                         
├── report.pdf                        # Technical report (IEEE format)
├── src/
│   ├── LoopLICMPass.cpp              # Main LICM pass implementation
│   ├── CMakeLists.txt                # CMake build configuration
│   └── build.sh                      # Build script
└── tests/
    ├── test1_simple.c                # Basic invariant test
    ├── test2_nested.c                # Nested loop test
    ├── test3_complex.c               # Dependency chain test
    ├── test4_dependency.c            # Negative test (loop-variant)
    ├── test5_safety.c                # Safety verification test
    ├── run_tests.sh                  # Test execution script
    └── analyze_results_advanced.sh   # Performance analysis script
```

## Algorithm Overview

### Optimization Strategy

The LICM pass operates in three phases:

1. **Invariant Identification**: Recursively identifies instructions whose operands are constants, defined outside the loop, or are themselves loop-invariant.

2. **Safety Analysis**: Uses LLVM's `isSafeToSpeculativelyExecute()` to prevent hoisting instructions that may trap (division by zero, overflow) or have side effects.

3. **Code Motion**: Moves safe invariant instructions to the loop preheader, ensuring they execute exactly once before the loop begins.

### Optimization Pipeline

```
mem2reg → instcombine → loop-simplify → loop-rotate → simple-licm → dce
```

- **mem2reg**: Promotes stack variables to SSA registers
- **instcombine**: Simplifies algebraic expressions
- **loop-simplify**: Ensures every loop has a preheader
- **loop-rotate**: Transforms loops into do-while style
- **simple-licm**: Our custom LICM pass
- **dce**: Removes dead code after optimization

## Implementation Highlights

### Key Design Decisions

- **Conservative Safety**: Excludes memory operations and function calls to guarantee correctness
- **Post-order Loop Traversal**: Processes inner loops before outer loops for nested optimization
- **Two-phase Transformation**: Separates candidate collection from code motion to avoid iterator invalidation
- **Speculation Safety**: Leverages LLVM's built-in safety checks to prevent runtime exceptions

## Performance Results

The optimization demonstrates measurable improvements across diverse test scenarios:

| Test Case | Execution Time | Speedup | Code Size Change |
|-----------|---------------|---------|------------------|
| test1_simple | 360ms → 120ms | 3.0× | -144 bytes |
| test2_nested | 770ms → 170ms | 4.5× | -144 bytes |
| test3_complex | 510ms → 330ms | 1.5× | -144 bytes |
| test4_dependency | 580ms → 410ms | 1.4× | -160 bytes |
| test5_safety | 140ms → 140ms | 1.0× | +0 bytes |

Compile Time Overhead: Negligible (0-2ms increase)  
Memory Usage: Constant at 1280 KB across all tests

## References

- [LLVM Pass Development Guide](https://llvm.org/docs/WritingAnLLVMPass.html)
- [Writing an LLVM Pass: 101](https://llvm.org/devmtg/2019-10/slides/Warzynski-WritingAnLLVMPass.pdf)
- Muchnick, S. S. *Advanced Compiler Design and Implementation*. Morgan Kaufmann, 1997.
- Aho, A. V., et al. *Compilers: Principles, Techniques, and Tools*. 2nd ed. Pearson, 2006.

## Acknowledgments

This project was completed as part of the Advanced Compiler Design course (CSIE 5054) at National Taiwan University. Special thanks to the course instructors and TAs for their guidance throughout the assignment.