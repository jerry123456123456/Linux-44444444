#include<sys/mman.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

int main(){
    int fd;
    char *mapped_area;
    struct stat sb;
    off_t file_size;
    
    //打开文件
    fd = open("example.txt",O_RDWR | O_CREAT);
    if(fd == -1){
        perror("open");
        return 1;
    }

    //获取文件大小
    if(fstat(fd,&sb) == -1){
        perror("fstat");
        close(fd);
        return 1;
    }
    file_size = sb.st_size;

    //将文件映射到内存
    /*
    addr：
    该参数指定映射的起始地址。
    如果传入 NULL，则表示让系统自动选择一个合适的地址来开始映射。这是比较常见的做法，因为系统可以根据当前内存使用情况选择一个未被使用的地址区域。
    length：
    该参数指定映射的长度，也就是要映射到内存中的字节数。
    在你提供的代码中，file_size 是文件的大小，因此映射的长度就是文件的大小，即映射整个文件到内存中。
    prot：
    该参数指定映射区域的保护模式，即对映射区域的访问权限。
    PROT_READ 表示映射区域可以被读取。
    PROT_WRITE 表示映射区域可以被写入。
    通过使用 | 运算符将 PROT_READ 和 PROT_WRITE 组合起来，表示映射区域既可以读取也可以写入。
    flags：
    该参数指定映射的标志，用于控制映射的行为。
    MAP_SHARED 表示此映射是共享的，对映射区域的修改会反映到文件中，并且多个进程可以共享此映射。如果多个进程映射了同一个文件的相同部分，它们将看到彼此的修改。
    其他可能的标志还有 MAP_PRIVATE，表示此映射是私有的，对映射区域的修改不会反映到文件中，而是会创建一个副本，后续的修改只会影响该副本。
    fd：
    该参数是文件描述符，用于标识要映射的文件。
    该文件描述符通常是通过 open 系统调用打开文件时得到的。在你的代码中，fd 是之前打开 example.txt 文件时获得的文件描述符。
    offset：
    该参数指定从文件的哪个位置开始映射，通常是从文件的起始位置开始。
    在你提供的代码中，0 表示从文件的起始位置（即偏移量为 0）开始映射。
    */
    mapped_area = mmap(NULL,file_size,PROT_READ | PROT_WRITE,MAP_SHARED,fd,0);
    if(mapped_area == MAP_FAILED) {
        perror("mmap");
        close(fd);
        return 1;
    }

    //对映射的内存区域进行操作，例如修改数据
    mapped_area[0] = 'A';
    mapped_area[1] = 'B';
    mapped_area[2] = 'C';

    //使用msync确保修改的数据同步到文件
    if(msync(mapped_area,file_size,MS_SYNC) == -1){
        perror("msync");
        munmap(mapped_area, file_size);
        close(fd);
        return 1;
    }

    //解除映射
    if(munmap(mapped_area,file_size) == -1){
        perror("munmap");
        close(fd);
        return 1;
    }

    //关闭文件
    close(fd);
    return 0;

    return 0;
}