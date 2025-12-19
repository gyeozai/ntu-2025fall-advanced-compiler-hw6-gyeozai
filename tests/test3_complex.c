// Test 3: Multiple Invariants
// 混合多個計算，有些可優化，有些不行

#include <stdio.h>

int complex_calc(int *arr, int size, int factor) {
    int total = 0;
    int x = 50;
    int y = 25;

    for (int i = 0; i < size; i++) {
        // Invariant 1: (x * y) -> Should be hoisted
        int const_part = x * y;
        
        // Invariant 2: (const_part / factor) -> Should be hoisted
        int scaling = const_part / factor;
        
        // Variant: arr[i] changes every iteration -> Cannot hoist
        // Variant: total changes -> Cannot hoist
        total += arr[i] + scaling;
    }
    
    return total;
}

int main() {
    int size = 100000000; // 1 億
    int arr_dummy[100];   // 為了省記憶體，我們用 mod 重複存取
    
    // Init array
    for(int k=0; k<100; k++) arr_dummy[k] = 1;

    int result = 0;
    // 為了模擬大陣列，我們用小陣列重複跑
    // 注意：這裡的重點是上面的 complex_calc 函數
    // 這裡我們直接傳入很大 size，但 arr 存取要小心越界，
    // 所以我們修改上面的函數為 arr[i % 100] 來避免 crash 
    // 但為了讓 IR 乾淨，我們假設 arr 足夠大，或是在這裡做一個 fake loop
    
    // 重寫上面的邏輯模擬：
    int total = 0;
    int x = 50;
    int y = 25;
    int factor = 2;
    
    for (int i = 0; i < 300000000; i++) { // 3 億次
        // 優化目標：這兩行數學運算應該被搬出去
        int const_part = x * y;
        int scaling = const_part / factor;
        
        total += (i % 10) + scaling;
    }
    
    printf("Result: %d\n", total);
    return 0;
}