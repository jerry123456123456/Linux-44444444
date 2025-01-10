#include <linux/module.h>
#include <linux/init.h>
#include <linux/percpu.h>
#include <linux/cpumask.h>

//#define DEFINE_PER_CPU(type, val) type val
//只在每个cpu副本中分配内存分配内存
static DEFINE_PER_CPU(long,cpuvar) = 5;
//cpualloc是一个指向每 CPU 变量的指针，类型为long，初始时为NULL
//__percpu 是一个用于声明每个 CPU 核心拥有自己独立副本的修饰符，特定于 Linux 内核。
static long __percpu *cpualloc;

static int __init my_init(void);
static void __exit my_exit(void);

static int __init my_init(void){
    int cpu;
    pr_info("module loaded at 0x%p\n", my_init);

    //for_each_possible_cpu宏用于遍历所有可能的 CPU。
    //参数cpu作为传出参数供内部成员使用
    for_each_possible_cpu(cpu){
        //per_cpu宏用于获取指定 CPU 上的cpuvar变量，并将其值设置为 10
        per_cpu(cpuvar,cpu) = 10;
        //get_cpu_var获取当前 CPU 上的cpuvar变量的值，并打印出来
        pr_info("init: cpuvar on cpu%d = %ld\n",
                cpu, get_cpu_var(cpuvar));
        //put_cpu_var释放对当前 CPU 变量的引用（用于防止 SMP 系统中的并发问题）
        /*
        当一个线程在 CPU 核心 A 上执行时，它可能访问了 CPU 核心 A 上的 per_cpu 变量。当它被调度到 CPU 核心 B 上时，可能需要访问 CPU 核心 B 上的 per_cpu 变量副本。
        虽然每个 CPU 上有独立的副本，但在 CPU 核心之间切换时，访问和同步这些副本的状态可能需要一些机制来保证正确性。
        在多核系统中，调度器会频繁地在多个 CPU 核心间调度线程，切换时如果没有合适的引用计数管理，可能导致访问当前 per_cpu 变量时出现错误
        （如：切换到其他 CPU 后仍然持有之前的引用，导致不一致或资源泄漏）。
        */
        put_cpu_var(cpuvar);
    }

    //__this_cpu_write用于在当前 CPU 上写入cpuvar的值为15
    __this_cpu_write(cpuvar,15);

    //alloc_percpu函数用于分配一个每 CPU 变量，返回一个指向每 CPU 变量的指针
    cpualloc = alloc_percpu(long);
    for_each_possible_cpu(cpu){
        //per_cpu_ptr宏用于获取指定 CPU 上的cpualloc变量的指针
        *per_cpu_ptr(cpualloc,cpu) = 666;
        pr_info("init: cpu:%d cpualloc = %ld\n",
                cpu, *per_cpu_ptr(cpualloc, cpu));
    }
    return 0;
}

static void __exit my_exit(void){
    int cpu;
    pr_info("exit module...\n");

    for_each_possible_cpu(cpu)
    {
        pr_info("cpuvar cpu%d = %ld\n", cpu, per_cpu(cpuvar, cpu));
        pr_info("exit: cpualloc%d = %ld\n", cpu, *per_cpu_ptr(cpualloc, cpu));
    }

    free_percpu(cpualloc);
    pr_info("Bye: module unloaded from 0x%p\n", my_exit);
}

module_init(my_init);
module_exit(my_exit);
MODULE_AUTHOR("jerry");
MODULE_LICENSE("GPL");
