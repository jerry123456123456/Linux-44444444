//不连续页分配器

#include <linux/module.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/kernel.h>

static int __init my_module_init(void);
static void __exit my_module_exit(void);

//模块初始化函数
static int __init my_module_init(void){
    //使用malloc分配一段虚拟连续的内存空间，这里分配4kb
    void *vaddr = vmalloc(4096);
    if (vaddr) {
        printk(KERN_INFO "Successfully allocated virtual contiguous memory at %p\n", vaddr);
        // 可以将这段内存空间当做缓冲区等进行使用，这里简单地将内存清零
        memset(vaddr, 0, 4096);

        // 模拟对这块内存的一些操作，比如当做字符数组写入数据
        char *str = (char *)vaddr;
        strcpy(str, "Hello, Non-contiguous Memory!");
        printk(KERN_INFO "Data written to the memory: %s\n", str);
        
        // 使用完毕后，需要通过vfree函数释放这块内存
        vfree(vaddr);
        printk(KERN_INFO "Memory has been freed.\n");
    } else {
        printk(KERN_ERR "Failed to allocate virtual contiguous memory.\n");
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