#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/mm.h>
#include <asm/page.h>  // 使用 <asm/page.h> 来获取 PAGE_SIZE 相关定义，不同架构可能有区别，这里先按常见方式

MODULE_LICENSE("GPL");

// 自定义模块初始化函数
static int __init my_module_init(void)
{
    struct vm_struct *area;
    void *virt_addr;

    // 使用get_vm_area来分配一个不连续的虚拟内存区域
    // 按照正确参数顺序，第一个参数传入要分配的内存大小（这里使用PAGE_SIZE），第二个参数传入分配标志 VM_ALLOC
    area = get_vm_area(PAGE_SIZE, VM_ALLOC);
    if (!area) {
        printk(KERN_ALERT "Failed to allocate vm area\n");
        return -ENOMEM;
    }

    // 获取分配的虚拟内存区域的起始虚拟地址
    virt_addr = area->addr;
    printk(KERN_INFO "Allocated virtual address: %p\n", virt_addr);

    // 可以在这里对这个虚拟内存区域进行一些使用操作，比如简单地设置值
    // 以下只是示例，实际应用中按具体需求操作
    memset(virt_addr, 0, 4096);

    // 释放之前分配的虚拟内存区域，使用 vunmap 函数来释放通过 get_vm_area 分配的虚拟内存区域
    vunmap(virt_addr);
    return 0;
}

// 自定义模块退出函数
static void __exit my_module_exit(void)
{
    printk(KERN_INFO "Exiting module\n");
}

module_init(my_module_init);
module_exit(my_module_exit);