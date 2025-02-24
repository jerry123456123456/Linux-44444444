// #include <linux/mm.h>
// #include <linux/module.h>
// #include <linux/sched.h>
// #include <linux/init.h>

// int __init VmaPages_InitFunc(void);
// void __exit VmaPages_ExitFunc(void);

// int __init VmaPages_InitFunc(void){
//     //当前进程的地址空间
//     struct mm_struct *pointer_mm = current->mm;
//     //指向当前进程虚拟区间中的某一个地址
//     unsigned long address = pointer_mm->mmap->vm_start + 1;
//     printk("Prompt:address=0x%lx\n",address);

//     //查到地址address所属的线性区间
//     struct vm_area_struct *pointer_vma = find_vma(pointer_mm,address);
//     if(pointer_vma == NULL){
//         printk("pointer_vma->vm_start=0x%lx\n",pointer_vma->vm_start);
//         printk("pointer_vma->vm_end=0x%lx\n",pointer_vma->vm_end);

//         //获取线性区间pointer_vma所包含的页数
//         //计算一个虚拟内存区域包含的页面数量
//         int pages_number = vma_pages(pointer_vma);

//         //输出线性区间pointer_vma的页数
//         printk("Prompt:pages_number=%d\n",pages_number);
//     }else{
//         printk("Prompt:pointer_vma failed to pointer to linearinterval.\n");
//     }
//     return 0;
// }

// void __exit VmaPages_ExitFunc(void){
//     printk("Prompt:Normal exit of kernel module.\n");
// }

// module_init(VmaPages_InitFunc); // 内核模块入口函数
// module_exit(VmaPages_ExitFunc); // 内核模块退出函数
// MODULE_LICENSE("GPL"); // 模块的许可证声明
// MODULE_DESCRIPTION("VMA pages"); // 模块描述


#include <linux/mm.h>
#include <linux/module.h>
#include <linux/sched.h>
#include <linux/init.h>

int __init VmaPages_InitFunc(void);
void __exit VmaPages_ExitFunc(void);

int __init VmaPages_InitFunc(void){
    // 当前进程的地址空间
    struct mm_struct *pointer_mm = current->mm;
    // 指向当前进程虚拟区间中的某一个地址
    unsigned long address = pointer_mm->mmap_base + 1;
    printk("Prompt:address=0x%lx\n",address);

    // 查到地址 address 所属的线性区间
    struct vm_area_struct *pointer_vma = find_vma(pointer_mm,address);
    if(pointer_vma!= NULL){
        printk("pointer_vma->vm_start=0x%lx\n",pointer_vma->vm_start);
        printk("pointer_vma->vm_end=0x%lx\n",pointer_vma->vm_end);

        // 获取线性区间 pointer_vma 所包含的页数
        // 计算一个虚拟内存区域包含的页面数量
        int pages_number = vma_pages(pointer_vma);

        // 输出线性区间 pointer_vma 的页数
        printk("Prompt:pages_number=%d\n",pages_number);
    }else{
        printk("Prompt:pointer_vma failed to pointer to linearinterval.\n");
    }
    return 0;
}

void __exit VmaPages_ExitFunc(void){
    printk("Prompt:Normal exit of kernel module.\n");
}

module_init(VmaPages_InitFunc); // 内核模块入口函数
module_exit(VmaPages_ExitFunc); // 内核模块退出函数
MODULE_LICENSE("GPL"); // 模块的许可证声明
MODULE_DESCRIPTION("VMA pages"); // 模块描述