#if 0
// //这个是linux5.x版本的
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mm_types.h>

int __init FindVma_InitFunc(void);
void __exit FindVma_ExitFunc(void);

int __init FindVma_InitFunc(void){
    struct mm_struct *mm;   //内核描述符指针变量
    unsigned long address;

    //定义虚拟内存区域的指针变量
    struct vm_area_struct *vmap;

    //current指向当前进程描述符指针
    //mm指向struct mm_struct结构体指针
    mm = current->mm;

    //获取当前进程下一个内存映射区域的起始地址，并将其加一
    address = mm->mmap->vm_next->vm_start + 1;

    // 输出该地址（ address:用户空间中的某一虚拟地址，当前进程第二个虚拟区间的起始地址加1）
    printk("Prompt:addrss = 0x%lx\n",address);

    vmap = find_vma(mm,address);
    if(vmap!=NULL){
        printk("Prompt:vmap->vm_start = 0x%lx\n",vmap->vm_start);
        printk("Prompt:vmap->vm_end = 0x%lx\n",vmap->vm_end);
    }
    else{
        printk("Prompt:Call find_vma() funtcion failed.\n");
    }

    return 0;
}

void __exit FindVma_ExitFunc(void)
{
    printk("Prompt:Normal exit module.\n");
}

module_init(FindVma_InitFunc); // 内核模块入口函数
module_exit(FindVma_ExitFunc); // 内核模块退出函数
MODULE_LICENSE("GPL"); // 模块的许可证声明
MODULE_AUTHOR("jerry"); // 声明由那一位作者或机构单位所编写的模块

#elif 1

// // 这个是linux6.x版本的
#include <linux/mm.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/mm_types.h>

int __init FindVma_InitFunc(void);
void __exit FindVma_ExitFunc(void);

int __init FindVma_InitFunc(void) {
    struct mm_struct *mm;    // 内核描述符指针变量
    unsigned long address;

    // 定义虚拟内存区域的指针变量
    struct vm_area_struct *vmap;

    // current指向当前进程描述符指针
    // mm指向struct mm_struct结构体指针
    mm = current->mm;

    // 通过mm->mmap_base访问虚拟内存区域链表
    vmap = find_vma(mm, mm->mmap_base);  // 通过mmap_base获取链表

    if (vmap != NULL) {
        // 获取第一个虚拟内存区域
        address = vmap->vm_start + 1;

        // 输出该地址（address: 用户空间中的某一虚拟地址，当前进程第一个虚拟区间的起始地址加1）
        printk("Prompt: address = 0x%lx\n", address);

        // 使用 find_vma 函数查找虚拟内存区域
        vmap = find_vma(mm, address);
        if (vmap != NULL) {
            printk("Prompt: vmap->vm_start = 0x%lx\n", vmap->vm_start);
            printk("Prompt: vmap->vm_end = 0x%lx\n", vmap->vm_end);
        } else {
            printk("Prompt: Call find_vma() function failed.\n");
        }
    } else {
        printk("Prompt: Current process has no memory mappings.\n");
    }

    return 0;
}

void __exit FindVma_ExitFunc(void) {
    printk("Prompt: Normal exit module.\n");
}

module_init(FindVma_InitFunc); // 内核模块入口函数
module_exit(FindVma_ExitFunc); // 内核模块退出函数
MODULE_LICENSE("GPL"); // 模块的许可证声明
MODULE_AUTHOR("jerry"); // 声明由哪一位作者或机构单位所编写的模块

#endif




