// Test 5: Safety Check
// 目標：測試可能導致 Crash 的指令是否被安全處理

#include <stdio.h>

int safety_test(int n, int d) {
    int sum = 0;
    
    // 這個迴圈可能一次都不會執行 (當 n=0)
    for (int i = 0; i < n; i++) {
        // [Trap] 除法運算可能導致除以零例外
        // 如果 n=0 (迴圈不進)，但我們把 x = 100 / d 搬到外面
        // 且剛好 d=0，程式就會在迴圈外 Crash，即使原本邏輯不該執行這行。
        int x = 100 / d; 
        sum += x;
    }
    return sum;
}

int main() {
    printf("Running Safe Test...\n");
    // Case 1: d=0, n=0。
    // 原本程式：迴圈不進，不會除以零 -> 活著
    // 錯誤優化：把除法搬出來 -> 除以零 -> Crash
    int res1 = safety_test(0, 0); 
    
    printf("Running Normal Test...\n");
    // Case 2: 正常執行
    int res2 = safety_test(100000000, 5);
    
    printf("Result: %d, %d\n", res1, res2);
    return 0;
}