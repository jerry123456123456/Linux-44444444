#include <linux/module.h>
#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/kernel.h>

// 模块初始化函数
static int __init my_module_init(void) {
    // 记录要分配内存的大小，方便后续计算结束地址等操作
    size_t size = 50 * sizeof(char);
    // 使用kmalloc分配一段内存，这里分配能存放长度为50的字符串的内存空间大小（包含'\0'）
    void *ptr = kmalloc(size, GFP_KERNEL);
    if (ptr) {
        printk(KERN_INFO "Successfully allocated memory using kmalloc at %p\n", ptr);

        // 将分配的内存当做字符数组使用
        char *str = (char *)ptr;
        strcpy(str, "Hello from kmalloc!");
        printk(KERN_INFO "Copied string to the allocated memory: %s\n", str);

        // 通过虚拟地址找到对应的struct page结构体指针，先获取起始地址对应的page
        struct page *start_page = virt_to_page(ptr);

        // 获取对应的物理地址起始地址（使用新的方式，通过page_to_pfn获取页帧号，再转换为物理地址）
        unsigned long phys_start_addr = PFN_PHYS(page_to_pfn(start_page));
        printk(KERN_INFO "The physical start address of the kmalloc'ed memory is: 0x%lx\n", phys_start_addr);

        // 计算物理地址的结束地址（假设内存连续，这里简单通过起始物理地址加上内存大小来计算）
        unsigned long phys_end_addr = phys_start_addr + size;
        printk(KERN_INFO "The physical end address of the kmalloc'ed memory is: 0x%lx\n", phys_end_addr);

        // 使用完毕后，通过kfree函数释放内存
        kfree(ptr);
        printk(KERN_INFO "Memory allocated by kmalloc has been freed.\n");
    } else {
        printk(KERN_ERR "Failed to allocate memory using kmalloc.\n");
        return -ENOMEM;
    }
    return 0;
}


// 模块退出函数
static void __exit my_module_exit(void) {
    printk(KERN_INFO "Module is exiting.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");