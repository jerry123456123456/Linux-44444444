#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>  //提供用户空间和内核空间之间数据拷贝的函数，如 copy_to_user 和 copy_from_user
#include <linux/miscdevice.h>  //用于创建杂项设备的结构体和函数
#include <linux/device.h>
#include <linux/slab.h>
#include <linux/kfifo.h>

#define CHRDEVICE_NAME "MY_ChrDevice_TEST"
static struct device *mydrv_device;
static char *device_buffer;

// 此宏定义将设备缓冲区最大尺寸限制在 40KB
#define MAX_DEVICE_BUFFER_SIZE (10*PAGE_SIZE)

//定义一个 ioctl 命令 MYDEV_CMD_GET_BUFSIZE，用于获取设备缓冲区大小。
//设备驱动程序将其解释为用户想要获取设备缓冲区的大小
#define MYDEV_CMD_GET_BUFSIZE 1

//打开设备
static int drv_open(struct inode *inode,struct file *file){
    int major = MAJOR(inode->i_rdev);  //对应文件所属驱动程序的主设备号
    int minor = MINOR(inode->i_rdev);  //对应文件所属驱动程序的次设备号
    printk("Prompt:%s:major=%d,minor=%d\n", __func__, major, minor);
    return 0;
}

// 释放设备
static int drv_release(struct inode *inode, struct file *file){
    return 0;
}

//从设备读数据
static ssize_t drv_read(struct file *file,char __user *buf,size_t count,loff_t *ppos){
    //用于从一个给定的缓冲区中读取数据。它通常用于简化设备驱动程序或内核模块中对用户空间数据的读取操作，尤其是在实现字符设备时
    /*
    buf：指向目标缓冲区的指针，这是用来存储从内核空间读取的数据的缓冲区。
    count：要读取的最大字节数，即从内核空间中复制到目标缓冲区的字节数限制。
    ppos：文件偏移量的指针，用于跟踪当前的读取位置。每次调用此函数时，偏移量会根据读取的字节数增加。
    vaddr：指向内核空间的源数据缓冲区的指针，即从中读取数据的地方。
    size：源缓冲区的大小，表示 vaddr 指向的内存块的总大小。
    */
    //device_buffer  ==>  buf  
    int ibytes = simple_read_from_buffer(buf,count,ppos,device_buffer,MAX_DEVICE_BUFFER_SIZE);
    printk("Prompt:%s: read ibytes=%d done at pos=%d\n", __func__, ibytes, (int)*ppos);
    return ibytes;
}

// 向设备写入数据
static ssize_t drv_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos){
    int ibytes = simple_write_to_buffer(device_buffer, MAX_DEVICE_BUFFER_SIZE, ppos, buf, count);
    printk("Prompt:%s: write ibytes=%d done at pos=%d\n", __func__, ibytes, (int)*ppos);
    return ibytes;
}

//将设备映射到用户空间地址以进行直接存储器访问DMA
/*
filp 是一个指向 struct file 的指针，代表打开的文件对象。

vma 是一个指向 struct vm_area_struct 的指针，它描述了用户空间中的虚拟内存区域，
包含了映射的各种信息，如起始地址（vma->vm_start）、结束地址（vma->vm_end）和页面偏移（vma->vm_pgoff）等。
*/
static int drv_mmap(struct file *filp,struct vm_area_struct *vma){
    unsigned long pfn;  //物理页帧号，用于将虚拟地址映射到物理地址
    //offset 是映射的偏移量，计算方式是 vma->vm_pgoff << PAGE_SHIFT。PAGE_SHIFT 是一个内核常量，通常为 12（因为页面大小通常是 4KB，即  字节），将页面偏移量转换为字节偏移量
    //将页面偏移量转为字节偏移量
    unsigned long offset = vma->vm_pgoff << PAGE_SHIFT; 
    //len 是要映射的长度，计算方式是 vma->vm_end - vma->vm_start，即用户请求映射的虚拟内存区域的长度
    unsigned long len = vma->vm_end - vma->vm_start;

    if (offset >= MAX_DEVICE_BUFFER_SIZE)
        return -EINVAL;
    if (len > (MAX_DEVICE_BUFFER_SIZE - offset))
        return -EAGAIN;

    printk("Prompt:%s: mapping %ld bytes of device buffer at offset %ld\n", __func__, len, offset);

    //virt_to_phys 是一个内核函数，将虚拟地址转换为物理地址
    // 使用 page_to_pfn 获取页面帧号
    pfn = page_to_pfn(virt_to_page(device_buffer + offset));

    //remap_pfn_range 是一个内核函数，用于将物理页帧号映射到用户空间的虚拟地址范围
    if(remap_pfn_range(vma,vma->vm_start,pfn,len,vma->vm_page_prot))
        return -EAGAIN;

    return 0;
}

//执行设备特定的I/O控制命令
/*
filp (struct file *):
filp 是指向 struct file 结构体的指针，表示正在操作的文件描述符。这个结构体包含了文件的状态、文件操作函数指针等信息。
在 ioctl 系统调用中，filp 通常用于访问文件的内部状态或特定信息，通常你不需要直接使用它，除非你想获取更多文件相关的信息（如文件位置、权限等）。

cmd (unsigned int cmd):
cmd 是一个整型值，表示要执行的控制命令。不同的命令会被映射到不同的处理逻辑。在这个函数中，cmd 对应的值会用于判断是否执行特定的操作。
例如，MYDEV_CMD_GET_BUFSIZE 是自定义的命令，指示驱动返回缓冲区的大小。

arg (unsigned long arg):
arg 是一个无符号长整型值，它通常用来传递命令的参数。对于不同的 cmd 命令，arg 的含义可以不同，可能是一个指针、一个数值或一个标志。
在这个函数中，arg 被转换为 void __user * 类型的 ioargp，这是为了访问用户空间的数据。
*/
static long drv_unlocked_ioctl(struct file *filp,unsigned int cmd,unsigned long arg){
    unsigned long tbs = MAX_DEVICE_BUFFER_SIZE;
    /*
    将 arg 转换为 void __user * 类型并赋值给 ioargp。__user 是一个宏，标记 ioargp 是指向用户空间的指针，表明它指向的是用户空间传递给内核的数据。
    arg 是一个长整型值，可以是用户空间传递的地址，这里将它转换为用户空间指针 ioargp，方便后续操作。
    */
    void __user *ioargp = (void __user *)arg;

    switch (cmd)
    {
    default:
        return -EAGAIN;
    case MYDEV_CMD_GET_BUFSIZE:
        if (copy_to_user(ioargp, &tbs, sizeof(tbs)))
            return -EFAULT;
        return 0;
    }
}

// 文件操作集
static const struct file_operations drv_fops = {
   .owner = THIS_MODULE,
   .open = drv_open,       // 打开设备
   .release = drv_release, // 释放设备
   .read = drv_read,       // 从设备读取数据
   .write = drv_write,     // 向设备写入数据
   .mmap = drv_mmap,       // 将设备映射到用户空间地址，进行直接存储器访问（DMA）
   .unlocked_ioctl = drv_unlocked_ioctl // 执行设备特定的 I/O 控制命令
};

// 定义结构体类型描述杂项设备
static struct miscdevice My_miscdevice = {
   .minor = MISC_DYNAMIC_MINOR, // 次设备号
   .name = CHRDEVICE_NAME,    // 设置名称 
   .fops = &drv_fops,        // 文件操作指针
};

//模块初始化函数
static int __init CharDevice_ModuleInitialization_Func(void){
    int result;

    device_buffer = kmalloc(MAX_DEVICE_BUFFER_SIZE, GFP_KERNEL);
    if (!device_buffer)
        return -ENOMEM;

    result = misc_register(&My_miscdevice);
    if (result){
        // 注册设备故障
        printk("Prompt：Register device failure.\n");
        kfree(device_buffer);
        return result;
    }

    mydrv_device = My_miscdevice.this_device;
    printk("Prompt：Character device registration successful, device:%s\n", CHRDEVICE_NAME);

    return 0;
}

// 模块卸载函数
static void __exit CharDevice_ModuleUninitialization_Func(void)
{
    // 释放内核内存
    kfree(device_buffer);
    // 注销字符设备
    misc_deregister(&My_miscdevice);
    // 移除字符设备驱动
    printk("Prompt:Successfully removed character device.\n");
}

module_init(CharDevice_ModuleInitialization_Func); // 内核模块入口函数
module_exit(CharDevice_ModuleUninitialization_Func); // 内核模块退出函数
MODULE_LICENSE("GPL"); // 模块的许可证声明
MODULE_DESCRIPTION("Character device instance"); // 模块描述
MODULE_AUTHOR("jerry"); // 声明由那一位作者或机构单位所编写的模块