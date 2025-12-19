// Test 4: Data Dependency (Negative Test)
// Objective: Ensure instructions dependent on modified variables are NOT hoisted.

#include <stdio.h>

int dependency_test(int start, int count) {
    int sum = 0;
    int a = start;
    
    for (int i = 0; i < count; i++) {
        int b = a + 10; 
        sum += b;
        a++; 
    }
    return sum;
}

int main() {
    long long total = 0;
    for(int k=0; k<10000000; k++) {
        total += dependency_test(k % 100, 100);
    }
    printf("Result: %lld\n", total);
    return 0;
}