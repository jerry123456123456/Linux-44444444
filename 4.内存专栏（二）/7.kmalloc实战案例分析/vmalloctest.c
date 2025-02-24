#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

#define MEMORY_KMALLOC_MAXSIZE 4096
char *pointerMemory=NULL;

// 模块初始化函数设计
int __init KmallocInit_Func(void)
{
    pointerMemory=(char*)kmalloc(MEMORY_KMALLOC_MAXSIZE,GFP_KERNEL);

    if(pointerMemory==NULL)
    {
        printk("Prompt:Failed to allocate memory.\n");
    }
    else
    {
        printk("Pormpt: Successfully called the kmalloc function to allocate memory, address = 0x%lx\n",(unsigned long)pointerMemory);
        
    }


    return 0;
}

// 模块退出函数设计
void __exit KmallocExit_Func(void)
{
    if(pointerMemory!=NULL)
    {
        kfree(pointerMemory);
        printk("Prompt:Successfully Released memory block.\n");
    }

    printk("Prompt:Normal exit of kernel module.\n");
}

module_init(KmallocInit_Func);
module_exit(KmallocExit_Func);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ovoice 2023/07/02");






