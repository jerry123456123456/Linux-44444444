#include <linux/slab.h>
#include <linux/mm.h>
#include <linux/mempool.h>
#include <linux/module.h>
#include <linux/printk.h>


// 自定义结构体，将使用 kmem_cache 来分配和管理
struct my_struct {
    int data1;
    char data2[20];
};

static struct kmem_cache *my_cache;
static mempool_t *my_mempool;


static int __init my_module_init(void)
{
    // 创建一个 kmem_cache 对象，用于管理 struct my_struct 类型的对象
    my_cache = kmem_cache_create("my_struct_cache", sizeof(struct my_struct), 0, SLAB_HWCACHE_ALIGN, NULL);
    if (!my_cache) {
        printk(KERN_ERR "Failed to create kmem_cache\n");
        return -ENOMEM;
    }

    // 创建一个内存池，使用 my_cache 作为后备存储
    my_mempool = mempool_create(10, mempool_alloc_slab, mempool_free_slab, my_cache);
    if (!my_mempool) {
        printk(KERN_ERR "Failed to create mempool\n");
        kmem_cache_destroy(my_cache);
        return -ENOMEM;
    }

    int i;
    for (i = 0; i < 20; i++) {  // 尝试分配 20 次，超过了初始的 10 个元素
        struct my_struct *obj = mempool_alloc(my_mempool, GFP_KERNEL);
        if (!obj) {
            printk(KERN_ERR "Failed to allocate memory from mempool at iteration %d\n", i);
            break;
        }

        // 初始化分配的对象
        obj->data1 = i;
        snprintf(obj->data2, sizeof(obj->data2), "Hello, mempool! %d", i);

        // 打印分配的对象信息
        printk(KERN_INFO "Allocated object at iteration %d: data1 = %d, data2 = %s\n", i, obj->data1, obj->data2);

        // 释放分配的对象到内存池
        mempool_free(obj, my_mempool);
    }

    // 销毁内存池
    mempool_destroy(my_mempool);
    // 销毁 kmem_cache
    kmem_cache_destroy(my_cache);
    return 0;
}

static void __exit my_module_exit(void)
{
    // 模块退出时的清理工作，这里暂时没有额外的清理操作
}

module_init(my_module_init);
module_exit(my_module_exit);
MODULE_LICENSE("GPL");