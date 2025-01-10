#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>

int main() {
    char* message = "Hello, world!";
    size_t len = sizeof(char) * 14;

    // 创建一个匿名共享内存区域，大小为len字节
    void* addr = mmap(NULL, len, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);

    if (addr == MAP_FAILED) {
        perror("mmap");
        exit(EXIT_FAILURE);
    }

    // 将消息复制到共享内存区域中
    memcpy(addr, message, len);

    // 打印共享内存区域中的消息
    printf("%s\n", (char*) addr);

    // 解除映射并释放资源
    munmap(addr, len);

    return EXIT_SUCCESS;
}

