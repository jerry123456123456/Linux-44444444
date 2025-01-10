#include <linux/init.h>
#include <linux/module.h>
#include <linux/gfp.h>
#include <linux/mm_types.h>
#include <linux/mm.h>

static int __init AllocPages_initFunc(void);
static void __exit AllocPages_exitFunc(void);

struct page *allocpages = NULL;

#define ALLOC_ORDER 4

static int __init AllocPages_initFunc(void){
    allocpages = alloc_pages(GFP_KERNEL,ALLOC_ORDER);  //分配2的4次方个物理页
    if(NULL==allocpages)
    {
        return -1;
    }
    else
    {
        printk("alloc_pages Allocation succeeded.\n");
        printk("page_address(allocpages) = 0x%lx\n", (unsigned long)page_address(allocpages));
    }
    return 0;

}


static void __exit AllocPages_exitFunc(void){
    if(allocpages)
    {
        __free_pages(allocpages,ALLOC_ORDER);    //释放所分配的16个页
        printk("__free_pages is Release successful! \n");
    }
    printk("Prompt : The program exits normally\n");
}

module_init(AllocPages_initFunc);
module_exit(AllocPages_exitFunc);

MODULE_LICENSE("GPL"); 
MODULE_AUTHOR("jerry"); 
MODULE_DESCRIPTION(" kernel module : alloc_pages/free_pages"); 