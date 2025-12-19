// Test 2: Nested Loops
// 目標：測試是否能將內層迴圈的不變量提到外層

#include <stdio.h>

long long process_matrix(int n) {
    long long sum = 0;
    int a = 10;
    int b = 20;
    
    // Outer loop
    for (int i = 0; i < n; i++) {
        // Inner loop
        for (int j = 0; j < n; j++) {
            // [Optimization Target]
            // (a * b) 對於內層迴圈是 Invariant
            // (a * b + i) 對於內層迴圈也是 Invariant (因為 i 在內層不變)
            // 我們的 Pass 應該要把這些計算搬出內層迴圈
            int complex_invariant = (a * b) + (a * 2) + i;
            
            sum += (complex_invariant + j) % 5;
        }
    }
    
    return sum;
}

int main() {
    // 20000 * 20000 = 4 億次運算
    long long result = process_matrix(20000);
    printf("Result: %lld\n", result);
    return 0;
}