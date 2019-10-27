#include "func_def.h"

//每一项代表一页, 值为0则未使用, 值为100则被使用
static byte mem_map [ PAGING_PAGES ] = {0,};
int high_memory = 0;


void memcpy(char *dst, const char *src, int count, int size)
{
    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < size; j++)
        {
            *(dst + i * size + j) = *(src + i * size + j);
        }
    }
}

//start:起始地址, end:终止地址
void mem_init(int start, int end)
{
    int i;
    for(i = 0; i < PAGING_PAGES; i++)
    {
        mem_map[i] = USED;
    }
    high_memory = end;

    if(start < LOW_MEM) start = LOW_MEM;
    for(i = MAP_NR(start); i <= MAP_NR(end) ; i++)
    {
        mem_map[i] = 0;
    }
    return ;
}

//根据memmap打印显示哪些页是空闲的, 哪些页是在使用的
void mem_printmap(void)
{
    int i = 0, j = 0;
    while(j < PAGING_PAGES)
    {
        if(mem_map[j]!=mem_map[i])
        {
            prints("from ");printi(i);prints(" to ");printi(j-1);prints(" : ");
            if(mem_map[i]==USED) prints("used"); else prints("idle");
            printc('\n');
            i = j;
        }
        else
        {
            j++;   
        }
    }
    prints("from ");printi(i);prints(" to ");printi(j-1);prints(" : ");
    if(mem_map[i]==USED) prints("used"); else prints("idle");
    printc('\n');
}

void mem_calc(void)
{
    int i, j, k, free = 0;
    int *pg_tbl, *pg_dir = 0x70000;

    for(i = 0; i < PAGING_PAGES; i++)
        if(!mem_map[i]) free++;
    printf("%d pages free (of %d in total)\n", i, PAGING_PAGES);
    
    // 遍历除了页表页目录的其余页表项, 如果页面有效, 则统计有效页面数量
    for(i = 2; i < 1024; i++) {
        if(pg_dir[i] & 1) {     // 先检查 Dir 是否存在
            pg_tbl = (int *)(0xfffff000 & pg_dir[i]);  // 计算 pg_tbl 的地址
            for(j = k = 0; j < 1024; j++) {
                if(pg_tbl[j] & 1) {     // 检查 Entry 是否存在
                    k++;
                }
            }
            printf("PageDir[%d] uses %d pages\n", i, k);
        } 
    }
    return ;
}

//此函数用于获得一个空闲的页面, 返回值为内存页面对应的起始地址
uint mem_getfreepage(void) {
   register uint __res asm("ax");
   __asm__ volatile ("std; repne; scasb\n\t"
                "jne 1f\n\t"
                "movb $1, 1(%%edi)\n\t"
                "sall $12, %%ecx\n\t"
                "addl %2, %%ecx\n\t"
                "movl %%ecx, %%edx\n\t"
                "movl $1024, %%ecx\n\t"
                "leal 4092(%%edx), %%edi\n\t"
                "rep; stosl;\n\t"
                "movl %%edx, %%eax\n"
                "1: cld"
                : "=a" (__res)
                : "0" (0), "i" (LOW_MEM), "c" (PAGING_PAGES), "D" (mem_map + PAGING_PAGES - 1));    // 从尾端开始检查是否有可用的物理页
   return __res;
}


void mem_functest(void)
{
    mem_init(0x100000,0x0898f00);
    int i = mem_getfreepage();
    printhex(i);
    mem_calc();
loop:
    for(;;);
}