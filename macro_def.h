#ifndef MACRO_DEF_H
#define MACRO_DEF_H

typedef unsigned int uint;
typedef unsigned char byte;

// define about "va" is for printf
typedef char *va_list;

/* Amount of space required in an argument list for an arg of type TYPE.
   TYPE may alternatively be an expression whose type is used.  */

#define __va_rounded_size(TYPE) \
    (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#define va_start(AP, LASTARG) \
    (AP = ((char *)&(LASTARG) + __va_rounded_size(LASTARG)))

#define va_arg(AP, TYPE) \
    (AP += __va_rounded_size(TYPE), *((TYPE *)(AP - __va_rounded_size(TYPE))))

// VGA 80x25x16 text mode
#define VIDEO_MEM 0xB8000
#define VIDEO_MEM_SIZE 4000  // 80 * 25 * 2
#define VIDEO_X_SZ 80
#define VIDEO_Y_SZ 25

// GDT set up in setup.S
#define GDT_ADDR 0x59700  // GDT_SEG_ADDR in setup.S
#define GDTR_LIMIT 0x6000 // GDTR_LIMIT in setup.S, 3072 Descriptor at most

#define IDT_ADDR 0x60000
#define IDT_INTR_LIMIT 256 // number of interrupts at most

// used when setting PIC
#define PIC0_ICW1 0x0020
#define PIC0_OCW2 0x0020
#define PIC0_IMR 0x0021
#define PIC0_ICW2 0x0021
#define PIC0_ICW3 0x0021
#define PIC0_ICW4 0x0021
#define PIC1_ICW1 0x00a0
#define PIC1_OCW2 0x00a0
#define PIC1_IMR 0x00a1
#define PIC1_ICW2 0x00a1
#define PIC1_ICW3 0x00a1
#define PIC1_ICW4 0x00a1

// used in timer init
#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

// cli and sti, use like function
#define io_cli() __asm__("cli\n\t")
#define io_sti() __asm__("sti\n\t")

// used in terminal.c
#define MAX_TERMINAL_CNT 16

// used in struct_def.h
#define BYTE_BUFFER_SIZE 32
#define TIMER_NUM 300

// used in time.c
#define MAX_TIME 0x7fffffff
#define TIMER_FLAGS_ALLOC 1
#define TIMER_FLAGS_USING 2

// used in mem_manage.c
// 0x00000000 - 0x000fffff为低1M空间, 用于放置内核
// 0x01000000 - 0x10000000共15M, 为空闲物理页面(其实内存更大, 扩展可以以后再做)
#define LOW_MEM 0x100000
#define PAGING_MEMORY (15 * 1024 * 1024)      // 空闲物理页面所占用的内存大小
#define PAGING_PAGES (PAGING_MEMORY >> 12)    // 空闲物理页面分页后的页数
#define MAP_NR(addr) (((addr)-LOW_MEM) >> 12) // 计算指定物理地址对应的页号
#define USED 100                              // 表示页面处于被占用状态
//该宏用于复制一页物理内存从 from 到 to
#define copy_page(from, to)                                     \
    __asm__("cld ; rep ; movsl" ::"S"(from), "D"(to), "c"(1024) \
            :)

//用于使TLB失效, 刷新缓存
#define invalidate() \
    __asm__ volatile("mov %%eax, %%cr3" ::"a"(0))

#endif
