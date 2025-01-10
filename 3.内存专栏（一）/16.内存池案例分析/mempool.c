#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//表示每次分配内存的一个基本单位大小
#define ALLOC_SIZE 8   
//通常用于控制循环的次数
#define LOOP 5
//定义了宏 MAX_POOL_SIZE，它的值是 1024 * 1024（也就是 1MB），这个宏主要用于限制内存池的最大可分配大小
#define MAX_POOL_SIZE 1024*1024
//表示内存池中每个内存块的大小
#define BLOCK_SIZE 64

//内存映射表
typedef struct memory_map_table
{   
    //一个指针，用于指向内存池中具体的某一个内存块，通过这个指针可以找到对应的实际内存块位置，方便后续对该内存块进行读写等操作
    char *p_block;
    //一般用来记录该内存块对应的索引，比如在内存池以数组形式管理内存块时，这个索引可以方便地定位到该内存块在整个内存块集合中的位置，便于查找、遍历等操作
    int index;
    //通常用于标记该内存块是否已经被使用了，比如值为 0 表示未使用，值为 1 表示已被分配出去正在使用
    int used;
}Memory_Map_Table;

//内存分配表
typedef struct memory_alloc_table
{   
    //是一个指针，指向此次内存分配所涉及的内存起始位置，也就是分配出去的那一片连续内存区域的开头地址
    char *p_start;
    //同样是用于标记该分配记录是否正在被使用
    int used;
    //表示此次分配所涉及的内存块在内存池中对应的起始索引
    int block_start_index;
    //记录了此次内存分配所涉及的内存块的数量，例如一次分配可能占用了多个连续的内存块
    int block_cnt;
}Memory_Alloc_Table;

// /对整个内存池相关信息的一个整合封装
typedef struct memory_pool
{
    char *memory_start;   //内存池起始地址,通过这个指针就可以找到内存池的整体位置
    Memory_Alloc_Table *alloc_table;
    Memory_Map_Table *map_table;
    //记录了整个内存池总的大小，单位通常是字节等，用于在内存分配、释放等操作时判断是否超出内存池容量限制等情况
    int total_size;
    //表示内存池内部可用于分配的总内存大小（比如去除了一些用于管理结构等占用的空间后的实际可分配大小）
    int internal_total_size;
    //表示内存池在动态扩容等操作时每次增加的大小，例如当内存池空间不够用了，可能按照这个增量值去扩展内存池的容量
    int increment;
    //：记录了当前内存池中已经被使用的内存大小
    int used_size;
    //表示内存池中每个内存块的大小
    int block_size;
    //记录了内存池中内存块的总数量
    int block_cnt;
    //用于统计当前内存池中已经进行的内存分配次数
    int alloc_cnt;
}Memory_Pool;

//定位内存池中内存映射表相关的位置信息。
Memory_Map_Table *map_table_pos(Memory_Pool *pool){
    //是因为在内存布局中，可能紧跟着内存池结构体之后存放的就是内存映射表相关的数据，通过这样的偏移计算，就可以找到内存映射表所在的内存起始位置
    Memory_Map_Table *pm = (Memory_Map_Table *)(pool->memory_start + sizeof(Memory_Pool));
    return pm;
}

//其功能应该是定位内存池中内存分配表所在的位置
Memory_Alloc_Table *alloc_table_pos(Memory_Pool *pool){
    /*
    首先同样是从内存池的起始地址 pool->memory_start 开始，先加上 sizeof(Memory_Pool)，跳过内存池结构体本身占用的内存空间，
    然后再加上 sizeof(Memory_Map_Table)*(pool->block_cnt)，这里的 sizeof(Memory_Map_Table) 是内存映射表中单个结构体元素所占用的字节大小，
    乘以 pool->block_cnt（内存池中内存块的数量）表示内存映射表整体所占用的字节大小
    */
    Memory_Alloc_Table *pm=(Memory_Alloc_Table*)(pool->memory_start+sizeof(Memory_Pool)+sizeof(Memory_Map_Table)*(pool->block_cnt));
    return pm;
}

// /定位内存池中实际可用于分配的内存区域的起始位置（即除去管理结构等占用空间后真正存放数据的内存起始处）
char *memory_pos(Memory_Pool *pool)
{
    char *pm = (char *)(pool->memory_start + sizeof(Memory_Pool) +
            (sizeof(Memory_Map_Table) + sizeof(Memory_Alloc_Table)) * pool->block_cnt);  //它这种方式就相当于把分配表往多了算，最多是一块一分配
    return pm;
}

//内存池初始化
//size表示要初始化的内存池的期望大小，用于指定内存池可容纳的内存总量；increment 用于在后续内存池动态扩展等操作时，指示每次增加的内存大小
Memory_Pool *memory_pool_init(int size,int increment){
    //指向通过 malloc 函数申请的一块连续内存空间，这块内存空间后续会被用来构建整个内存池相关的结构体以及可分配内存区域等，初始化为 NULL，表示目前还没有指向有效的内存地址
    char *p = NULL;
    //主要用于辅助定位内存池中实际可分配内存区域的位置，初始化为 NULL
	char *p_memory = NULL;
    //内存池的类型
	Memory_Pool *pool = NULL;
    //内存分配表
	Memory_Alloc_Table *alloc_table = NULL;
    //用于在循环遍历内存分配表的各个元素时充当临时指针
	Memory_Alloc_Table *p_alloc_table = NULL;
    //内存映射表
	Memory_Map_Table *map_table = NULL;	
    //用于在循环遍历内存映射表元素时作为临时指针
	Memory_Map_Table *p_map_table = NULL;
    //用于记录内存池中内存块的数量
	int block_cnt = 0;
    //用于计算整个内存池（包括内存池结构体、内存映射表、内存分配表以及实际可分配内存区域等所有部分）总共需要占用的字节大小
	int all_size = 0;
	int i = 0;

    if (size < 0 || size > MAX_POOL_SIZE) {
        printf("memory_pool_init(): Invalid size(%d)\n", size);
        return pool;
    }

    //计算内存块数量和总内存大小部分
    //这里根据传入的期望内存池大小 size 和每个内存块的大小 BLOCK_SIZE（宏定义的值）来计算内存池中内存块的数量
    //例如，如果 size 为 65，BLOCK_SIZE 为 64，那么就需要 2 个内存块来容纳这些内存，而不是只取整得到 1 个内存块。
    block_cnt = ((size + BLOCK_SIZE - 1) / BLOCK_SIZE);
    //接着计算整个内存池需要占用的总字节大小
    all_size = sizeof(Memory_Pool) + (sizeof(Memory_Map_Table) +
            sizeof(Memory_Alloc_Table)) * block_cnt + size;

    p = (char *)malloc(all_size);
    if (p == NULL) {
        perror("Malloc failed\n");
        return pool;
    }
    memset(p, 0, all_size);

    //内存池结构体属性设置部分
    pool = (Memory_Pool *)p;
    pool->block_cnt = block_cnt;
    pool->block_size = BLOCK_SIZE;
    pool->increment = increment;
    pool->internal_total_size = BLOCK_SIZE * block_cnt;
    pool->total_size = size;
    pool->used_size = 0;
    pool->alloc_cnt = 0;
    pool->memory_start = p;

    //获取第四个区域，也就是实际内存位置
    p_memory = memory_pos(pool);
    //或群第二个区域，也就是内存映射表的位置
    map_table = map_table_pos(pool);
    for (i = 0; i < block_cnt; i++) {
        p_map_table = (Memory_Map_Table *)((char *)map_table + i * sizeof(Memory_Map_Table));
        p_map_table->index = 0;
        p_map_table->p_block = p_memory + i * BLOCK_SIZE;
        p_map_table->used = 0;
    }

    alloc_table=alloc_table_pos(pool);
	for (i = 0; i < block_cnt; i++) {
		p_alloc_table = (Memory_Alloc_Table *)((char *)alloc_table + i * sizeof(Memory_Alloc_Table));
		p_alloc_table->block_cnt = 0;
		p_alloc_table->block_start_index = -1;
		p_alloc_table->p_start = NULL;
		p_alloc_table->used = 0;
	}
 
	printf("memory_pool_init: total size: %d, block cnt: %d, block size: %d\n",
			pool->total_size, pool->block_cnt, BLOCK_SIZE);
	return pool;
}


void *memory_malloc(Memory_Pool *pool,int size){
    char *p_start = NULL;
    int need_block_cnt = 0;
    Memory_Alloc_Table *alloc_table = NULL;
    Memory_Alloc_Table *p_alloc_table = NULL;
    Memory_Map_Table *map_table = NULL;
    Memory_Map_Table *p_map_table = NULL;

    //用于记录在遍历内存映射表寻找空闲内存块过程中，连续找到的空闲内存块数量，初始化为 0
    int block_cnt = 0;
    //表示找到的可用于分配的连续空闲内存块的起始索引，初始化为 -1
    int start_index = -1;
    int i = 0;

    //参数错误
    if (size <= 0) {
        printf("Invalid size(%d)\n", size);
        return p_start;
    }

    //大于总内存
    if (size > pool->total_size) {
        printf("%d is more than total size\n", size);
        return p_start;
    }

    //大于剩余可用
    if (size > pool->total_size - pool->used_size) {
        printf("Free memory(%d) is less than allocate(%d)\n",
               pool->total_size - pool->used_size, size);
        return NULL;
    }

    //计算此次分配操作需要的内存块数量,这里采用了向上取整的计算方式
    need_block_cnt = (size + BLOCK_SIZE - 1) / BLOCK_SIZE;
    
    //定位内存映射表并查找空闲内存块部分
    map_table = map_table_pos(pool);

    start_index = -1;
    for (i = 0; i < pool->block_cnt; i++) {
        p_map_table = (Memory_Map_Table *)((char *)map_table + i * sizeof(Memory_Map_Table));
        if (p_map_table->used) {
            block_cnt = 0;
            start_index = -1;
            continue;
        }

        if (start_index == -1) {
            start_index = i;
        }
        block_cnt++;
        //直到分配够了,咱就撤
        if (block_cnt == need_block_cnt) {
            break;
        }
    }

    //经过前面查找空闲内存块的循环操作后，检查 start_index 的值，如果仍然为 -1，意味着没有找到足够的连续空闲内存块来满足此次分配请求
    if (start_index == -1) {
        printf("No available memory to used\n");
        return NULL;
    }

    //定位内存分配表并查找空闲记录部分
    //调用 alloc_table_pos 函数（用于定位内存分配表起始位置的函数），传入 pool 指针，获取内存分配表的起始地址
    alloc_table = alloc_table_pos(pool);
    for (i = 0; i < pool->block_cnt; i++) {
        p_alloc_table = (Memory_Alloc_Table *)((char *)alloc_table + i * sizeof(Memory_Alloc_Table));
        //判断当前遍历到的内存分配表元素中的 used 成员变量是否为 0，如果是，说明找到了一个空闲的内存分配记录，就使用 break 语句跳出循环，结束查找过程
        if (p_alloc_table->used == 0) {
            //就是说这个分配表没有顺序
            break;
        }
        p_alloc_table = NULL;
    }

    if (p_alloc_table == NULL) {
        return NULL;
    }

    //更新内存分配表和内存映射表相关记录部分
    //通过计算偏移量的方式，根据之前找到的空闲内存块起始索引 start_index，从内存映射表起始地址 map_table 开始，
    //定位到对应的内存映射表中起始的那个空闲内存块元素（也就是此次分配要使用的内存块在内存映射表中的第一个对应记录元素），
    //并将其地址赋值给 p_map_table，使得 p_map_table 指向这个元素，方便后续操作
    p_map_table = (Memory_Map_Table *)((char *)map_table + sizeof(Memory_Map_Table) * start_index);
    //将 p_map_table 所指向的内存映射表元素中的 p_block 成员（指向实际内存块的指针）赋值给 p_alloc_table 中的 p_start 成员，
    //这样内存分配表记录中就记录了此次分配的内存起始地址，建立起了内存分配表与实际分配的内存块之间的关联。
    p_alloc_table->p_start = p_map_table->p_block;
    p_alloc_table->block_start_index = p_map_table->index;
    p_alloc_table->block_cnt = block_cnt;
    p_alloc_table->used = 1;

    for (i = start_index; i < start_index + block_cnt; i++) {
        p_map_table = (Memory_Map_Table *)((char *)map_table + i * sizeof(Memory_Map_Table));
        p_map_table->used = 1;
    }

    //更新内存池相关统计信息及返回分配的内存地址部分
    printf("Alloc size: %d, Block: (start: %d, end: %d, cnt: %d)\n", size,
        start_index, start_index + block_cnt - 1, block_cnt);
    pool->alloc_cnt++;
    pool->used_size += size;
    return p_alloc_table->p_start;
}


void memory_free(Memory_Pool *pool, void *memory){
    Memory_Alloc_Table *alloc_table = NULL;
    Memory_Alloc_Table *p_alloc_table = NULL;
    Memory_Map_Table *map_table = NULL;
    Memory_Map_Table *p_map_table = NULL;

    int i = 0;
    //用于记录此次要释放的内存所涉及的内存块在内存池中对应的起始索引
    int block_start_index = 0;
    //此次释放涉及到的内存块数量
    int block_cnt = 0;
    int found = 0; // 新增一个标志变量，用于记录是否找到对应的内存分配表记录

    if (memory == NULL){
        printf("memory_free():memory is NULL.\n");
        return;
    }

    if (pool == NULL){
        printf("Pool is NULL.\n");
        return;
    }

    //查找内存分配表相关记录部分
    alloc_table = alloc_table_pos(pool);
    for (i = 0; i < pool->alloc_cnt; i++){
        p_alloc_table = (Memory_Alloc_Table *)((char *)alloc_table + i * sizeof(Memory_Alloc_Table));
        if (p_alloc_table->p_start == memory){
            block_start_index = p_alloc_table->block_start_index;
            block_cnt = p_alloc_table->block_cnt;
            found = 1; // 找到对应记录，设置标志变量为1
            break; // 找到后直接退出循环，提高效率
        }
    }

    //判断释放有有效记录的部分
    if (block_cnt == 0){
        return;
    }

    //更新内存映射表相关状态部分
    map_table = map_table_pos(pool);
    printf("Block:Free:start:%d,end:%d,cnt:%d\n", block_start_index, block_start_index + block_cnt - 1, block_cnt);
    for (i = block_start_index; i < block_start_index + block_cnt; i++){
        //从找到的要释放的索引开始
        p_map_table = (Memory_Map_Table *)((char *)map_table + i * sizeof(Memory_Map_Table));
        //设置为没有使用
        p_map_table->used = 0;
    }

    //更新内存分配表及内存池使用状态部分
    p_alloc_table->used = 0;
    pool->used_size -= block_cnt * BLOCK_SIZE;
}


int memory_pool_destroy(Memory_Pool **pool) // 修改参数为二级指针
{
    if (*pool == NULL)
    {
        printf("memory_pool_destroy:pool is NULL.\n");
        return 0; // 返回0表示销毁失败，因为传入的指针本身就是NULL
    }

    free(*pool);
    *pool = NULL;
    return 1; // 返回1表示销毁成功
}

int main()
{
    Memory_Pool *pool = NULL;
    char *p1 = NULL;
    char *p2 = NULL;
    int i = 0;

    pool = memory_pool_init(1024, 512);
    if (pool == NULL)
    {
        printf("Memory pool init Failed.\n");
        return -1; // 返回错误码，表示内存池初始化失败，程序异常退出
    }

    for (i = 0; i < 2; i++)
    {
        p1 = (char *)memory_malloc(pool, ALLOC_SIZE);
        if (p1 == NULL)
        {
            printf("malloc Failed.\n");
        }
        else
        {
            printf("Malloc success.\n");
            memory_free(pool, p1);
        }
    }

    // 调用内存池销毁函数，并根据返回值判断是否销毁成功
    if (memory_pool_destroy(&pool))
    {
        printf("Memory pool destroyed successfully.\n");
    }
    else
    {
        printf("Failed to destroy the memory pool.\n");
    }

    return 0;
}

