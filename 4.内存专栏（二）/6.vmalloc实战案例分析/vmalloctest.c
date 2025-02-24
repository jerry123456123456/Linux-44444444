#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/init.h>

// 定义指针变量指向所分配的内存区域
char *pointerMemory=NULL;

// 分配字节数量
#define MEMORY_VMALLOC_MAXSIZE 4096

// 模块初始入口函数
int __init VmallocInit_Func(void)
{
    pointerMemory=(char*)vmalloc(MEMORY_VMALLOC_MAXSIZE);

    if(NULL==pointerMemory)
    {
        printk("Prompt:Memory allocation failed.\n");
    }
    else
    {
        printk("Prompt:Memory allocation successfully, virtual address = 0x%lx\n",(unsigned long)pointerMemory);
    }

    return 0;
}

// 模块退出函数
void __exit VmallocExit_Func(void)
{
    //  判断pointerMemory指针变量是否为NULL
    if(NULL!=pointerMemory)
    {
        vfree(pointerMemory);
        printk("Prompt:Successfully called vfree function.\n");
    }

    printk("Prompt:Normal exit of kernel module.\n");
}

module_init(VmallocInit_Func);
module_exit(VmallocExit_Func);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Ovoice 2023/07/02");
