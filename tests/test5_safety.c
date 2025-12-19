// Test 5: Safety Check
// Objective: Verify that instructions with potential side effects (e.g., division) are handled safely.

#include <stdio.h>

int safety_test(int n, int d) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int x = 100 / d; 
        sum += x;
    }
    return sum;
}

int main() {
    printf("Running Safe Test...\n");
    int res1 = safety_test(0, 0); 
    
    printf("Running Normal Test...\n");
    int res2 = safety_test(100000000, 5);
    
    printf("Result: %d, %d\n", res1, res2);
    return 0;
}