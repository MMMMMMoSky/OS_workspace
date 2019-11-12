#include "macro_def.h"

#define MAX_PROCS 16

// #define FIRST_TSS_ENTRY 11
// #define FIRST_LDT_ENTRY (FIRST_TSS_ENTRY-1)
// #define _TSS(n) ((((unsigned long) n)<<4)+(FIRST_TSS_ENTRY<<3))
// #define _LDT(n) ((((unsigned long) n)<<4)+(FIRST_LDT_ENTRY<<3))
// #define ltr(n) __asm__("ltr %%ax"::"a" (_TSS(n)))
// #define lldt(n) __asm__("lldt %%ax"::"a" (_LDT(n)))

// #define move_to_user_mode() \
// __asm__ ("movl %%esp,%%eax\n\t" \
// 	"pushl $0x17\n\t" \
// 	"pushl %%eax\n\t" \
// 	"pushfl\n\t" \
// 	"pushl $0x0f\n\t" \
// 	"pushl $1f\n\t" \
// 	"iret\n" \
// 	"1:\tmovl $0x17,%%eax\n\t" \
// 	"movw %%ax,%%ds\n\t" \
// 	"movw %%ax,%%es\n\t" \
// 	"movw %%ax,%%fs\n\t" \
// 	"movw %%ax,%%gs" \
// 	:::"ax")


struct tss_struct {
	int	back_link;	/* 16 high bits zero */
	int	esp0;
	int	ss0;		/* 16 high bits zero */
	int	esp1;
	int	ss1;		/* 16 high bits zero */
	int	esp2;
	int	ss2;		/* 16 high bits zero */
	int	cr3;
	int	eip;
	int	eflags;
	int	eax,ecx,edx,ebx;
	int	esp;
	int	ebp;
	int	esi;
	int	edi;
	int	es;		/* 16 high bits zero */
	int	cs;		/* 16 high bits zero */
	int	ss;		/* 16 high bits zero */
	int	ds;		/* 16 high bits zero */
	int	fs;		/* 16 high bits zero */
	int	gs;		/* 16 high bits zero */
	int	ldt;		/* 16 high bits zero */
	int	trace_bitmap;	/* bits: trace 0, bitmap 16-31 */
};

struct desc_struct{
	unsigned int a,b;
};

// struct proc_struct{
// 	int state;
// 	int counter;
// 	int priority;
// 	//int signal;
// 	//...
// 	unsigned int start_code;
// 	unsigned int code_length;
// 	unsigned int data_length;
// 	unsigned int start_stack;
// 	int pid;
// 	struct proc_struct * p_father;
// 	//...
// 	struct desc_struct ldt[3];
// 	struct tss_struct tss;
// };


struct proc_struct_simple
{
	int used;
	int state;
	int priority;
	int selector;
	struct desc_struct ldt[3];
	struct tss_struct tss;
	int next;
};


struct proc_struct_simple proc_arr[MAX_PROCS];
int current;

void test_proc();
void wait_key();
void switch_proc();
