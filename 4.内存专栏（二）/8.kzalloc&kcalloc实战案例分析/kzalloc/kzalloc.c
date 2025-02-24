#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>  // 包含 kzalloc 的定义

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jerry");
MODULE_DESCRIPTION("A simple Linux driver using kzalloc");
MODULE_VERSION("1.0");

static int __init kzalloc_example_init(void);
static void __exit kzalloc_example_exit(void);

static int __init kzalloc_example_init(void){
    int *ptr;
    size_t size = 10 * sizeof(int);  //分配10个整数大小的内存

    //使用kzalloc分配内存并初始化为0
    ptr = kzalloc(size,GFP_KERNEL);
    if(NULL == ptr){
        printk("Prompt:kzalloc() function allocation failed.\n");
    }
    else{
        printk("Prompt:kzalloc() function allocation successfully,address = 0x%lx\n",(unsigned long)ptr);
        printk("Prompt:The content of ptr is : %d\n",*(ptr + 4));
    }

    pr_info("Memory allocated successfully using kzalloc\n");

    //使用分配的内存，简单打印
    for(int i = 0;i < 10;i++){
        pr_info("ptr[%d] = %d\n", i, ptr[i]);
    }

    // 释放分配的内存
    kfree(ptr);
    return 0;
}

static void __exit kzalloc_example_exit(void){
    pr_info("Exiting kzalloc example module\n");
}

module_init(kzalloc_example_init);
module_exit(kzalloc_example_exit);