#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>

#define MAX 1024

int main(int argc,char *argv[]){
    int *p = sbrk(0);
    int *old = p;
    printf("old: %p\n",old);

    p = sbrk(MAX * MAX);
    if(p == (void *)(-1)){
        perror("sbrk error.\n");
    }

    printf("old: %p\tp = %p\n",p,old);

    int *new = sbrk(0);
    printf("new: %p\n",new);

    printf("\npid = %d\n\n",getpid());

    while(1){
        sleep(1);
    }

    sbrk(-MAX * MAX);

    return 0;
}