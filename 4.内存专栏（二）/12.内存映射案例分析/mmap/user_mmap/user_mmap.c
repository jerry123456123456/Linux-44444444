#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>  //进程控制函数
#include <sys/mman.h>  //内存映射函数
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>  //I/O控制函数
#include <malloc.h>
#include <stdlib.h>

#define DEVICE_NAME "/dev/MY_ChrDevice_TEST"
#define MYDEV_CMD_GET_BUFFER 1 //定义虚拟FIFO设备缓冲区

int main(){
    int fd,i;
    size_t length;

    //测试虚拟FIFO设备
    char msg[]="Prompt：Testing the virtual FIFO device.\n";
    char *read_buffer,*mmap_buffer;
    length = sizeof(msg);

    fd = open(DEVICE_NAME,O_RDWR);
    if (fd < 0){
        printf("Prompt：open device %s failded.\n", DEVICE_NAME);
        return -1;
    }

    //控制设备或文件的函数
    //调用 ioctl 函数向文件描述符 fd 所代表的设备发送 MYDEV_CMD_GET_BUFFER 请求，期望设备驱动程序将设备的缓冲区大小存储在 length 所指向的变量中。
    if(ioctl(fd,MYDEV_CMD_GET_BUFFER,&length) < 0){
        printf("Prompt: icotl fail.\n");
        goto open_fail;
    }
    printf("Prompt: driver max buffer size=%ld\n", length);

    read_buffer = malloc(length);
    if (!read_buffer)
        goto open_fail;

    //使用 mmap 函数将设备的缓冲区映射到用户空间
    mmap_buffer = mmap(NULL,length,PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mmap_buffer == (char*)MAP_FAILED){
        goto map_fail;
    }
    printf("Prompt:mmap driver buffer successed: %p\n", mmap_buffer);

    // 修改 mmaped 缓冲区
    for (i = 0; i < length; i++){
        *(mmap_buffer + i) = (char)random();
    }

        // 读取缓冲区并与 mmap 缓冲区进行比较
    if (read(fd, read_buffer, length)!= length){
        printf("Prompt:read fail.\n");
        goto read_fail;
    }

    if (memcmp(read_buffer, mmap_buffer, length)){
        printf("Prompt:buffer compare fail.\n");
        goto read_fail;
    }

    printf("Prompt:data modify and compare successfully.\n");

    munmap(mmap_buffer, length);
    free(read_buffer);
    close(fd);  

    return 0;

open_fail:
    close(fd);
map_fail:
    free(read_buffer);
read_fail:
    munmap(mmap_buffer, length); 

    return -1;
}