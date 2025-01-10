#include <sys/mman.h>
#include <stdio.h>

int main() {
    // 创建一个匿名的共享内存区域
    void* addr = mmap(NULL, 4096, PROT_READ | PROT_WRITE, MAP_ANONYMOUS | MAP_SHARED, -1, 0);
    if (addr == MAP_FAILED) {
        perror("mmap");
        return -1;
    }

    // 在共享内存中写入数据
    char* str = (char*) addr;
    sprintf(str, "Hello world!");

    // 打印共享内存中的数据
    printf("%s\n", str);

    // 解除映射并释放资源
    munmap(addr, 4096);

    return 0;
}

