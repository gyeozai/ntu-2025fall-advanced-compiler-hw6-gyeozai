// Test 4: Data Dependency (Negative Test)
// 目標：證明 Pass 不會錯誤地搬移依賴於迴圈變數的指令

#include <stdio.h>

int dependency_test(int start, int count) {
    int sum = 0;
    int a = start;
    
    for (int i = 0; i < count; i++) {
        // [Trap] 這裡的 b 雖然看起來像 (a + 10)，
        // 但是 a 在下一行被修改了！
        // 如果 Pass 錯誤地把 b = a + 10 搬出去，結果就會錯。
        int b = a + 10; 
        
        sum += b;
        a++; // a 隨著迴圈改變，所以上面的 b 不是 Invariant
    }
    return sum;
}

int main() {
    // 跑多次一點確保時間可測，但重點是結果正確性
    long long total = 0;
    for(int k=0; k<10000000; k++) {
        total += dependency_test(k % 100, 100);
    }
    printf("Result: %lld\n", total);
    return 0;
}