#include "proc.h"
#include "func_def.h"
// 一下仅作为描述符结果验证
// #define _set_tssldt_desc(n,addr,type) \
// __asm__ ("movw $104,%1\n\t" \
// 	"movw %%ax,%2\n\t" \
// 	"rorl $16,%%eax\n\t" \
// 	"movb %%al,%3\n\t" \
// 	"movb $" type ",%4\n\t" \
// 	"movb $0x00,%5\n\t" \
// 	"movb %%ah,%6\n\t" \
// 	"rorl $16,%%eax" \
// 	::"a" (addr), "m" (*(n)), "m" (*(n+2)), "m" (*(n+4)), \
// 	 "m" (*(n+5)), "m" (*(n+6)), "m" (*(n+7)) \
// 	)
// #define set_tss_desc(n,addr) _set_tssldt_desc(((char *) (n)),addr,"0x89")
// #define set_ldt_desc(n,addr) _set_tssldt_desc(((char *) (n)),addr,"0x82")


struct proc_struct * proc[MAX_PROCS] = {};

void set_gdtdesc(int ind, int base, int limit, short flag, short access)
{
    int * add = (int *)(GDT_ADDR) + ind * 2;
    *add = (base << 16) | (limit & 0xffff);
    *(add + 1) = base & 0xff000000 | (flag & 0xf)<<20 | (limit & 0x000f0000) |
                 access << 8 | (base & 0x00ff0000) >> 16;
}

void setLdtDesc(int n, int base)
{
    set_gdtdesc(n, base, 110, 0, 0x82);
}

void setTssDesc(int n, int base)
{
    set_gdtdesc(n, base, 110, 0, 0x89);
}

void setDesc(struct desc_struct * p, int a, int b)
{
    p->a = a;
    p->b = b;
}

// void print_ldt(struct proc_struct * p)
// {
//     printf("start address:%x\n", &p->ldt);
//     for(int i=0;i<3;i++){
//         printf("%d: %x\n",i,p->ldt[i]);
//     }
// }

void sched_init(void)
{
    // // //设置第一个进程的进程控制块
    // struct proc_struct * p = ()0;//&init_proc;

    // //设置对应的ldt和tss
    // setLdtDesc(10, &p->ldt);
    // setTssDesc(11, &p->tss);

    // struct desc_struct * d = (struct desc_struct *)GDT_ADDR;
    // d += 12;
    // for(int i=1;i<MAX_PROCS;i++){
    //     proc[i] = (void *)(0); 
    //     d->a = d->b = 0;
    //     p++;
    //     d->a = d->b = 0;
    //     p++;
    // }
    // //复位NT标志位
    // __asm__("pushfl ; andl $0xffffbfff,(%esp) ; popfl" );

    // lldt(0);
    // ltr(0);
}


// void create_process(int i)
// {
//     unsigned long old_data_base,new_data_base,data_limit;
// 	unsigned long old_code_base,new_code_base,code_limit;

// 	code_limit=get_limit(0x0f);
// 	data_limit=get_limit(0x17);
// 	old_code_base = get_base(current->ldt[1]);
// 	old_data_base = get_base(current->ldt[2]);

//     new_data_base = new_code_base = i * 0x4000000;
//     set_base(proc_arr[i].ldt[1], new_code_base);
//     set_base(proc_arr[i].ldt[2], new_data_base);
//     //...
// }


//上面的代码不要使用


void main_b()
{
    for(int i=0;;i++)
    {
        if(i%100000==0)
        {
            printf("%d ",proc_arr[current].priority);
        }
    }
}

int time_to_switch;
extern struct byte_buffer kb_buf;

void wait_key()
{
    printf("wait input");
    while(1){
        //byte data = get_byte_buffer(&kb_buf);  // 可以优化, 一次取多个
        if(kb_buf.length!=0) 
        {
            break;
        }
    }
}

void initFirstProc()
{
    //初始化所有任务的描述符以及tss段
    for(int i=0;i<MAX_PROCS;i++){
        proc_arr[i].selector = (i+10)*8;
        proc_arr[i].used = 0;

        proc_arr[i].tss.cr3 = 0x70000;
        proc_arr[i].tss.ldt = 0;
        proc_arr[i].tss.trace_bitmap = 0x40000000;
        setTssDesc((i+10), &(proc_arr[i].tss));
        //setLdtDesc((2*i+11), &(proc_arr[i].ldt));
        proc_arr[i].state = 0;
    }

    proc_arr[0].used = 1;
    proc_arr[0].state = 1;
    proc_arr[0].next = 0;
    proc_arr[0].priority = 1;
    time_to_switch = proc_arr[0].priority*1000;
    //加载第一个tss的选择符
    load_tr(10*8);
    current = 0;
}

int find_proc()
{
    for(int i=0;i<MAX_PROCS;i++){
        if(proc_arr[i].used==1)
            continue;
        else
            return i; 
    }
    return 0;//0表示没有找到，0一定被使用了，所以不会冲突
}

int new_proc(unsigned int addr, int priority)
{
    int i;
    if(!(i=find_proc()))
        return 0;

    proc_arr[i].priority = priority;
    proc_arr[i].tss.eip = (int) addr;
	proc_arr[i].tss.eflags = 0x00000606; /* IF = 1; */
	proc_arr[i].tss.eax = 0;
	proc_arr[i].tss.ecx = 0;
	proc_arr[i].tss.edx = 0;
	proc_arr[i].tss.ebx = 0;
	//proc_arr[i].tss.esp = 0x200000;
	proc_arr[i].tss.ebp = 0;
	proc_arr[i].tss.esi = 0;
	proc_arr[i].tss.edi = 0;
	proc_arr[i].tss.es = 2 * 8;
	proc_arr[i].tss.cs = 1 * 8;
	proc_arr[i].tss.ss = 2 * 8;
	proc_arr[i].tss.ds = 2 * 8;
	proc_arr[i].tss.fs = 2 * 8;
	proc_arr[i].tss.gs = 2 * 8;

    //关键是堆栈的设置
    proc_arr[i].tss.esp = 0x1000000 + 0x10000*(i+1);
    proc_arr[i].next = proc_arr[current].next;
    proc_arr[current].next = i;
    return i;
}

void kill_proc(int i)
{

}

void switch_proc()
{
    int j = proc_arr[current].next;
    if(j!=current) {
        current = j;
        time_to_switch = proc_arr[current].priority*100;
        //printf("%d", time_to_switch);
        farjmp(0, proc_arr[j].selector);
    }
    else {
        time_to_switch = proc_arr[current].priority*100;
        //printf("%d", time_to_switch);
        return;
    }
}

struct lock
{
    int locked;
    int pid;
};

struct lock lock_key;
struct lock lock_video;

void init_lock()
{
    lock_key.locked = 0;
    lock_video.locked = 0;
}

int holding(struct lock * lk)
{
    return (lk->locked == 1 && lk->pid == current);
}

static inline uint
xchg(volatile uint *addr, uint newval)
{
    uint result;
    // The + in "+m" denotes a read−modify−write operand.
    asm volatile("lock; xchgl %0, %1"
                 : "+m" (*addr), "=a" (result) 
                 : "1" (newval) 
                 : "cc");
    return result;
}

int get_lock(struct lock * lk)
{   
    if(holding(lk)){
        printf("error");
        for(;;);
    }
    
    while(xchg(&lk->locked,1)!=0);
    __sync_synchronize();

    lk->pid = current;
}

void test_proc()
{
    initFirstProc();
    int i;
    if(i = new_proc(main_b, 10))
    {
        printf("success");
    };

    for(int i=0;;i++) if(i%100000==0)printf("%d ",proc_arr[current].priority);
    for(;;);
}