#include<linux/module.h>
#include<linux/slab.h>
#include<linux/init.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jerry");
MODULE_DESCRIPTION("kmem_cache usage example");

struct my_struct{
    int a;
    char b[16];
};

static struct kmem_cache *my_cache = NULL;

static int __init my_module_init(void);
static void __exit my_module_exit(void);

static int __init my_module_init(void)
{
    struct my_struct *obj;

    //创建一个内存缓存，分配struct my_struct类型的对象
    my_cache = kmem_cache_create("my_struct_cache",sizeof(struct my_struct),0,SLAB_HWCACHE_ALIGN,NULL);

    if (!my_cache) {
        pr_err("Failed to create kmem_cache\n");
        return -ENOMEM;
    }

    pr_info("kmem_cache created successfully\n");

    //从内存缓存中分配一个对象
    obj = kmem_cache_alloc(my_cache,GFP_KERNEL);
    if (!obj) {
        pr_err("Failed to allocate object from kmem_cache\n");
        kmem_cache_destroy(my_cache);
        return -ENOMEM;
    }

    pr_info("Object allocated from kmem_cache: %p\n", obj);

    //初始化分配的对象
    obj->a = 42;
    snprintf(obj->b, sizeof(obj->b), "Hello, kmem_cache!");

    pr_info("Object initialized: a=%d, b=%s\n", obj->a, obj->b);
    
    //释放对象回到内存
    kmem_cache_free(my_cache,obj);

    pr_info("Object freed to kmem_cache\n");

    return 0;
}

static void __exit my_module_exit(void)
{
    // 销毁内存缓存
    if (my_cache) {
        kmem_cache_destroy(my_cache);
        pr_info("kmem_cache destroyed\n");
    }
}

module_init(my_module_init);
module_exit(my_module_exit);