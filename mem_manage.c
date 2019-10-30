#include "func_def.h"

//每一项代表一页, 值为0则未使用, 值为100则被使用
static byte mem_map[PAGING_PAGES] = {
    0,
};
int high_memory = 0;

void memcpy(byte *dst, const byte *src, uint count)
{
    while (count--) {
        *(dst + count) = *(src + count);
    }
}

//start:起始地址, end:终止地址
void mem_init(int start, int end)
{
    int i;
    for (i = 0; i < PAGING_PAGES; i++)
    {
        mem_map[i] = USED;
    }
    high_memory = end;

    if (start < LOW_MEM)
        start = LOW_MEM;
    for (i = MAP_NR(start); i <= MAP_NR(end); i++)
    {
        mem_map[i] = 0;
    }
    return;
}

//根据memmap打印显示哪些页是空闲的, 哪些页是在使用的
void mem_printmap(void)
{
    int i = 0, j = 0;
    while (j < PAGING_PAGES)
    {
        if (mem_map[j] != mem_map[i])
        {
            prints("from ");
            printi(i);
            prints(" to ");
            printi(j - 1);
            prints(" : ");
            if (mem_map[i] == USED)
                prints("used");
            else
                prints("idle");
            printc('\n');
            i = j;
        }
        else
        {
            j++;
        }
    }
    prints("from ");
    printi(i);
    prints(" to ");
    printi(j - 1);
    prints(" : ");
    if (mem_map[i] == USED)
        prints("used");
    else
        prints("idle");
    prints("\n\n");
}

void mem_calc(void)
{
    int i, j, k, free = 0;
    int *pg_tbl, *pg_dir = 0x70000;

    for (i = 0; i < PAGING_PAGES; i++)
        if (!mem_map[i])
            free++;
    printf("%d pages free (of %d in total)\n", free, PAGING_PAGES);

    // 遍历除了页表页目录的其余页表项, 如果页面有效, 则统计有效页面数量
    for (i = 2; i < 1024; i++)
    {
        if (pg_dir[i] & 1)
        {                                             // 先检查 Dir 是否存在
            pg_tbl = (int *)(0xfffff000 & pg_dir[i]); // 计算 pg_tbl 的地址
            for (j = k = 0; j < 1024; j++)
            {
                if (pg_tbl[j] & 1)
                { // 检查 Entry 是否存在
                    k++;
                }
            }
            printf("PageDir[%d] uses %d pages\n", i, k);
        }
    }
    printf("\n");
    return;
}

//此函数用于获得一个空闲的页面, 返回值为内存页面对应的起始地址
uint mem_getfreepage(void)
{
    register uint __res asm("ax");
    __asm__ volatile(
        "std\n\t"
        "repne\n\t"
        "scasb\n\t"
        "jne 1f\n\t"
        "movb $1, 1(%%edi)\n\t"
        "sall $12, %%ecx\n\t"
        "addl %2, %%ecx\n\t"
        "movl %%ecx, %%edx\n\t"
        "movl $1024, %%ecx\n\t"
        "leal 4092(%%edx), %%edi\n\t"
        "rep\n\t"
        "stosl\n\t"
        "movl %%edx, %%eax\n"
        "1: cld"
        : "=a"(__res)
        : "0"(0), "i"(LOW_MEM), "c"(PAGING_PAGES), "D"(mem_map + PAGING_PAGES - 1)); // 从尾端开始检查是否有可用的物理页
    return __res;
}

//释放给定物理地址的页面
void mem_freepage(uint addr)
{
    if (addr < LOW_MEM)
        return;
    if (addr >= high_memory)
        printf("trying to free nonexistent page");
    addr -= LOW_MEM;
    addr >>= 12;
    if (mem_map[addr]--)
        return;
    mem_map[addr] = 0;
    printf("trying to free free page");
}

/*
 * 以下函数用于内核中的内存分配与释放(参考linux0.11mallo库函数))
 * 这个算法还有点小复杂！
*/
#define PAGE_SIZE 4096
struct bucket_desc *free_bucket_desc = (struct bucket_desc *)0;

typedef struct _bucket_dir s_bucket_dir;
static struct _bucket_dir bucket_dir[10];

//用于初始化一页用于分配桶
void init_bucket()
{
    struct bucket_desc *bdesc, *first;
    bdesc = first = (struct bucket_desc *)mem_getfreepage();
    if (!bdesc)
    {
        printf("no idle page!");
        return;
    }
    for (int i = PAGE_SIZE / sizeof(struct bucket_desc); i > 1; i--)
    {
        bdesc->next = bdesc + 1;
        bdesc++;
    }
    bdesc->next = free_bucket_desc;
    free_bucket_desc = first;
}

void *mem_alloc(uint len)
{
    struct bucket_desc *bdesc;
    struct _bucket_dir *bdir;
    void *ret;
    for (bdir = &bucket_dir[0]; bdir->size; bdir++)
    {
        if (bdir->size >= len)
        {
            break;
        }
    }
    if (!bdir->size)
    {
        printf("try to allocate too much memory!");
        return;
    }

    io_cli();
    for (bdesc = bdir->chain; bdesc; bdesc = bdesc->next)
        if (bdesc->freeptr)
            break;

    if (!bdesc)
    {
        char *cp;

        if (!free_bucket_desc)
            init_bucket();

        bdesc = free_bucket_desc;
        free_bucket_desc = bdesc->next;
        bdesc->bucket_size = bdir->size;
        bdesc->refcnt = 0;
        bdesc->page = bdesc->freeptr = cp = mem_getfreepage();

        if (!bdesc->page)
        {
            printf("no idle pages!");
        }
        for (int i = 1; i < PAGE_SIZE / bdir->size; i++)
        {
            *((char **)cp) = cp + bdir->size;
            cp += bdir->size;
        }

        *((char **)cp) = 0;
        bdesc->next = bdir->chain;
        bdir->chain = bdesc;
    }
    ret = bdesc->freeptr;
    bdesc->freeptr = *((char **)ret);
    bdesc->refcnt++;
    io_sti();
    return ret;
}

void mem_free(void *obj, uint size)
{
    struct _bucket_dir *bdir;
    struct bucket_desc *bdesc, *prev;
    for (bdir = bucket_dir; bdir->size; bdir++)
    {
        prev = 0;
        if (bdir->size < size)
            continue;
        for (bdesc = bdir->chain; bdesc; bdesc++)
        {
            if (bdesc->page = obj)
                goto found;
            prev = bdesc;
        }
    }
    printf("bad address!");
found:
    io_cli();
    *((char **)obj) = bdesc->freeptr;
    bdesc->freeptr = obj;
    bdesc->refcnt--;
    if (!bdesc->refcnt)
    {
        if (prev->next != bdesc)
        {
            for (prev = bdir->chain; prev; prev = prev->next)
                if (prev->next == bdir)
                    break;

            if (prev)
                prev->next = bdesc->next;
            else
            {
                if (bdir->chain != bdesc)
                    printf("malloc bucket chains corrupted");
                bdir->chain = bdesc->next;
            }
        }
        mem_freepage((uint)bdesc->page);
        bdesc->next = free_bucket_desc;
        free_bucket_desc = bdesc;
    }
}

void mem_print_freebucket()
{
    int i = 0;
    struct bucket_desc *bdesc = free_bucket_desc;
    while (bdesc != 0)
    {
        printf("%d ", i++);
        bdesc = bdesc->next;
    }
}

void mem_print_bucketdir()
{
    for (int i = 0; i < 9; i++)
    {
        printf("%d:", bucket_dir[i].size);
        for (struct bucket_desc *bdesc = bucket_dir[i].chain; bdesc; bdesc = bdesc->next)
        {
            printf("(%x %d %x) ", bdesc->page, bdesc->refcnt, bdesc->freeptr);
        }
        printf("\n");
    }
}

void mem_initbdir()
{
    int sz = 16;
    for (int i = 0; i < 10; i++)
    {
        bucket_dir[i].size = 1 << (i + 4);
        bucket_dir[i].chain = (struct bucket_desc *)0;
    }
    bucket_dir[9].size = 0;
    return;
}

// 用于测试内存管理函数
void mem_functest(void)
{
    mem_init(0x100000, 0x0898f00);
    mem_initbdir();
    void *addr = mem_alloc(100);
    void *addr2 = mem_alloc(1000);
    void *addr3 = mem_alloc(512);
    void *addr4 = mem_alloc(10);
    void *addr5 = mem_alloc(48);

    mem_print_bucketdir();
    mem_free(addr, 0);
    mem_free(addr2, 0);
    mem_free(addr3, 0);
    mem_free(addr4, 0);
    mem_free(addr5, 0);

    mem_print_bucketdir();
loop:
    for (;;)
        ;
}