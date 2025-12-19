// Test 2: Nested Loops
// Objective: Test if invariants in inner loops are successfully hoisted to the outer loop.

#include <stdio.h>

long long process_matrix(int n) {
    long long sum = 0;
    int a = 10;
    int b = 20;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            int complex_invariant = (a * b) + (a * 2) + i;
            sum += (complex_invariant + j) % 5;
        }
    }
    
    return sum;
}

int main() {
    long long result = process_matrix(20000);
    printf("Result: %lld\n", result);
    return 0;
}