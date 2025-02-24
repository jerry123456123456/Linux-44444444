#include <linux/module.h>
#include <linux/slab.h>
#include <linux/init.h>

struct kmem_cache *pointer_mycache=NULL;

int __init Kmemcachecreate_InitFunc(void);
void __exit Kmemcachecreate_ExitFunc(void);

//内核模块初始化函数
int __init Kmemcachecreate_InitFunc(void){
    //创建一个slab缓存
    pointer_mycache = kmem_cache_create("mycache_test",64,0,SLAB_HWCACHE_ALIGN,NULL);

    if(NULL == pointer_mycache){
         printk("Prompt:Call kmem_cache_create() failed to create cache.\n");
    }
    else{
        printk("Prompt:cache size is = %d\n",kmem_cache_size(pointer_mycache));
    }
    return 0;
}

void __exit Kmemcachecreate_ExitFunc(void){
    if(pointer_mycache)
    {
        kmem_cache_destroy(pointer_mycache);
        printk("Prompt:call kmem_cache_destroy() successfully released slab cache.\n");
    }

    printk("Prompt:Normal exit of kernel module.\n");
}

module_init(Kmemcachecreate_InitFunc);
module_exit(Kmemcachecreate_ExitFunc);
MODULE_LICENSE("GPL");
