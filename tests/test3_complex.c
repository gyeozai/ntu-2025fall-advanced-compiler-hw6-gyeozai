// Test 3: Multiple Invariants
// Objective: Verify hoisting of multiple calculations while ensuring variants remain in the loop.

#include <stdio.h>

int complex_calc(int *arr, int size, int factor) {
    int total = 0;
    int x = 50;
    int y = 25;

    for (int i = 0; i < size; i++) {
        int const_part = x * y;
        int scaling = const_part / factor;
        total += arr[i] + scaling;
    }
    
    return total;
}

int main() {
    int size = 100000000; 
    int arr_dummy[100];   
    
    for(int k=0; k<100; k++) arr_dummy[k] = 1;

    int total = 0;
    int x = 50;
    int y = 25;
    int factor = 2;
    
    for (int i = 0; i < 300000000; i++) { 
        int const_part = x * y;
        int scaling = const_part / factor;
        total += (i % 10) + scaling;
    }
    
    printf("Result: %d\n", total);
    return 0;
}