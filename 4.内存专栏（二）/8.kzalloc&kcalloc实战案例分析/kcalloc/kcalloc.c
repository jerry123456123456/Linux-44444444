#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>  // 包含 kcalloc 的定义

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jerry");
MODULE_DESCRIPTION("A simple Linux driver using kcalloc");
MODULE_VERSION("1.0");


static int __init kcalloc_example_init(void);
static void __exit kcalloc_example_exit(void);

static int __init kcalloc_example_init(void){
    int *ptr;
    size_t num_elements = 10;  // 分配 10 个元素
    size_t element_size = sizeof(int);  // 每个元素大小是 int

    //使用kcalloc分配内存并初始化为0
    ptr = kcalloc(num_elements,element_size,GFP_KERNEL);

    if(NULL == ptr){
        printk("Prompt:kcalloc() function allocation failed.\n");
    }
    else{
        printk("Prompt:kcalloc() function allocation successfully,address = 0x%lx\n",(unsigned long)ptr);
        printk("Prompt:The content of ptr is : %d\n",*(ptr + 4));
    }

    pr_info("Memory allocated successfully using kcalloc\n");

    // 使用分配的内存，简单打印每个值
    for (int i = 0; i < num_elements; i++) {
        pr_info("ptr[%d] = %d\n", i, ptr[i]);
    }

    // 释放分配的内存
    kfree(ptr);
    return 0;
}

static void __exit kcalloc_example_exit(void){
    pr_info("Exiting kcalloc example module\n");
}


module_init(kcalloc_example_init);
module_exit(kcalloc_example_exit);