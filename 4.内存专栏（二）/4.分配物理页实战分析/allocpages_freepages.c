#include <linux/init.h>
#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/mm_types.h>
#include <linux/mm.h>


// 表示物理内存页的结构体类型（物理内存页直接被组成成一人厚度似于链表的数据结构（即页框链表））
struct page *pages=NULL; 

static int __init allocpages_InitFunc(void);
static void __exit allocpages_ExitFunc(void);

static int __init allocpages_InitFunc(void)
{
    pages=alloc_pages(GFP_KERNEL,4); //  分配16个物理页面
    if(!pages)
    {
        printk("Prompt:Not enough avilable memory.\n");
        // 错误代码，表示没有足够的可用内存
        // 返回此错误：意味着系统已经耗尽可用的物理内存或虚拟地址空间
        return -ENOMEM; 
    }
    else
    {
        printk("Prompt:Successfully assigned physical page\n");
        printk("Prompt:page_address(page) = 0x%lx\n",(unsigned long)page_address(pages));    
    }

    return 0;
}

static void __exit allocpages_ExitFunc(void)
{
    if(pages)
    {
        // 释放所分配的16个物理页面
        __free_pages(pages,4);
        printk("Prompt:Successfully released physical page.\n");
    }

    printk("Prompt:Exit Module.\n");
}

module_init(allocpages_InitFunc); // 内核模块入口函数
module_exit(allocpages_ExitFunc); // 内核模块退出函数
MODULE_LICENSE("GPL"); // 模块的许可证声明
MODULE_AUTHOR("0voice 2023/07/02"); // 声明由那一位作者或机构单位所编写的模块



