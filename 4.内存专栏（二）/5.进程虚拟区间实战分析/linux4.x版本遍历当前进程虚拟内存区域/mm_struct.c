#include <linux/module.h>
#include <linux/init.h>
#include <linux/mm_types.h>
#include <linux/sched.h>

// 模块初始化函数
static int __init my_vma_print_init(void)
{
    struct task_struct *current_task;
    struct mm_struct *mm;
    struct vm_area_struct *vma;

    // 获取当前进程的task_struct结构体
    current_task = current;
    // 获取当前进程的内存描述符结构体
    mm = current_task->mm;

    if (mm) {
        // 遍历虚拟内存区域链表
        vma = mm->mmap;
        while (vma) {
            printk(KERN_INFO "VMA start: 0x%lx, end: 0x%lx, flags: 0x%lx\n",
                   (unsigned long)vma->vm_start,
                   (unsigned long)vma->vm_end,
                   (unsigned long)vma->vm_flags);
            vma = vma->vm_next;
        }
    }

    return 0;
}

// 模块退出函数
static void __exit my_vma_print_exit(void)
{
    printk(KERN_INFO "Module exiting\n");
}

module_init(my_vma_print_init);
module_exit(my_vma_print_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Your Name");
MODULE_DESCRIPTION("A module to print current process's VMAs");
