// Test 1: Simple Loop Invariant
// 目標：證明 (x * 5 + 10) 這種跟 i 無關的計算會被移出迴圈

#include <stdio.h>

// 防止編譯器太聰明把函數 inline 或直接算出結果
// 我們希望測量迴圈內的計算開銷
int compute(int input, int loop_count) {
    int sum = 0;
    int magic = input * 3; // 這是外部變數
    
    for (int i = 0; i < loop_count; i++) {
        // [Optimization Target]
        // (magic + 100) * 2 是一個 Loop Invariant
        // 原本每次迴圈都要算一次，優化後應該只算一次
        int invariant_calc = (magic + 100) * 2;
        
        // 為了避免被 Dead Code Elimination 刪掉，我們必須使用這個值
        sum += (invariant_calc + i) % 10;
    }
    
    return sum;
}

int main() {
    int input = 5;
    // 跑 2 億次，確保執行時間在秒級，這樣 time 指令才測得準
    int result = compute(input, 200000000); 
    
    printf("Result: %d\n", result);
    return 0;
}