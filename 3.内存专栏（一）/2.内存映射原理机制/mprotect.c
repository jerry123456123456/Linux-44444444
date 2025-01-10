#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

int main() {
    // 分配4096字节大小的内存空间
    void* ptr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap failed");
        exit(1);
    }
    
    // 输出分配的内存地址和初始值
    printf("Memory allocated at %p with value %d\n", ptr, *(int*)ptr);
    
    // 修改内存保护权限为只读 PROT_WRITE
    if (mprotect(ptr, 4096, PROT_WRITE) == -1) {
        perror("mprotect failed");
        exit(1);
    }
    
    // 尝试写入新值，会导致“段错误”（Segmentation fault）
    *(int*)ptr = 123;
    
    return 0;
}

