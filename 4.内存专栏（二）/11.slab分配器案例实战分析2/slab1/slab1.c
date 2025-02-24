#include <linux/module.h>
#include <linux/mm.h>
#include <linux/slab.h>
#include <linux/init.h>

static char *kmembuffer;

static int size = 10 * PAGE_SIZE;

//创建一个缓存对象
static struct kmem_cache *my_caches;

// 宏定义：用于将一个变量导出给用户空间，允许用户空间通过sysfs接口读取和修改此变量
// 第三参数0644指定sysfs文件系统中对应文件的权限
module_param(size,int,0644);

static int __init MySlab_InitFunc(void);
static void __exit MySlab_ExitFunc(void);

static int __init MySlab_InitFunc(void){
    /* 创建内存缓存 */
    if(size>KMALLOC_MAX_SIZE){
        printk("Prompt：size=%d is to large,you can't have more than %lu!\n",size,KMALLOC_MAX_SIZE);
        return -1;
    }

    //创建以及初始化内存高速缓存
    my_caches=kmem_cache_create("MyCacheTest",size,0,SLAB_HWCACHE_ALIGN,NULL);
    if(!my_caches){
        printk("Prompt：kmem_cache_create failed.\n");
        return -ENOMEM;
    }
    printk("Prompt：Create MyCacheTest successfully.\n");

    //分配内存缓存对象
    kmembuffer=kmem_cache_alloc(my_caches,GFP_ATOMIC);
    if(!kmembuffer){
        printk("Prompt :failed to create a cache object.\n");
        (void)kmem_cache_destroy(my_caches);
        return -1;
    }
    printk("Prompt：successfulyy created a object，kmembuffer address=0x%p\n",kmembuffer);
    
    return 0;
}

static void __exit MySlab_ExitFunc(void){
    /* 销毁内存缓存对象 */
    kmem_cache_free(my_caches,kmembuffer);
    printk("Prompt：destroyed a cache object.\n");

    /* 销毁内存缓存 */
    kmem_cache_destroy(my_caches);
    printk("Prompt：destroyed MyCacheTest.\n");
}

module_init(MySlab_InitFunc); // 内核模块入口函数
module_exit(MySlab_ExitFunc); // 内核模块退出函数
MODULE_LICENSE("GPL"); // 模块的许可证声明