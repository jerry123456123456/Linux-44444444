#include<linux/init.h>
#include<linux/module.h>
#include<linux/kernel.h>

#define ARRAY_SIZE 10

static int __init my_module_init(void){
    int array[ARRAY_SIZE];
    int i;

    //越界访问，会触发kasan检测
    for(i = 0;i <= ARRAY_SIZE;i++){
        array[i] = i;
    }

    printk(KERN_INFO "My module initialized.\n");
    return 0;
}

static void __exit my_module_exit(void)
{
    printk(KERN_INFO "My module exited.\n");
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("jerry");
MODULE_DESCRIPTION("A simple module with a memory error");