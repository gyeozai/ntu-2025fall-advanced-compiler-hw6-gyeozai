// Test 1: Simple Loop Invariant
// Objective: Verify that expressions independent of loop variable 'i' are hoisted.

#include <stdio.h>

int compute(int input, int loop_count) {
    int sum = 0;
    int magic = input * 3; 
    
    for (int i = 0; i < loop_count; i++) {
        int invariant_calc = (magic + 100) * 2;
        sum += (invariant_calc + i) % 10;
    }
    
    return sum;
}

int main() {
    int input = 5;
    int result = compute(input, 200000000); 
    
    printf("Result: %d\n", result);
    return 0;
}