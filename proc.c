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

int time_to_switch;
extern struct byte_buffer kb_buf;
struct lock lock_kb;
struct lock lock_video;
extern unsigned int video_mem;
extern uint cursor_x, cursor_y;
extern struct terminal * terminal_table[MAX_TERMINAL_CNT] ;
extern uint cur_term;   

struct proc_struct_simple proc_arr[MAX_PROCS];
int current;

void main_b()
{
    printf("bbbbbbbbbbb");  
    for(int i=0;;i++)
    {
        if(get_lock(&lock_kb))
            printf("get lock\n");
        
        while(1){
            if (kb_buf.length == 0) continue;
            io_cli();
            byte data = get_byte_buffer(&kb_buf);  // 可以优化, 一次取多个
            io_sti();
            printf("%d", data);
        }
    }
}

void main_a()
{
    printf("aaaaaaaaaaa");
    for(int i=0;;i++)
    {
        if(get_lock(&lock_kb))
            printf("get lock\n");
        while(1){
            if (kb_buf.length == 0) continue;
            io_cli();
            byte data = get_byte_buffer(&kb_buf);  // 可以优化, 一次取多个
            io_sti();
            printf("%d",data);
        }
    }
}


void wait_key()
{
    printf("wait input");
    while(1){ 
        if(kb_buf.length!=0) 
        {
            // /byte data = get_byte_buffer(&kb_buf);  // 可以优化, 一次取多个
            break;
        }
    }
}

void initFirstProc()
{
    //初始化所有任务的描述符以及tss段
    for(int i=0;i<MAX_PROCS;i++){
        proc_arr[i].selector = (i+10)*8;
        proc_arr[i].state = STA_END;

        proc_arr[i].tss.cr3 = 0x70000;
        proc_arr[i].tss.ldt = 0;
        proc_arr[i].tss.trace_bitmap = 0x40000000;
        setTssDesc((i+10), (int)(&(proc_arr[i].tss)));
        //setLdtDesc((2*i+11), &(proc_arr[i].ldt));
    }

    char name[] = "main";
    memcpy(proc_arr[0].name, name, 5);
    proc_arr[0].state = STA_WAKE;
    proc_arr[0].next = 0;
    proc_arr[0].prev = 0;
    proc_arr[0].priority = 1;
    time_to_switch = proc_arr[0].priority*1000;
    //加载第一个tss的选择符
    load_tr(10*8);
    current = 0;
}

int find_proc()
{
    for(int i=0;i<MAX_PROCS;i++){
        if(proc_arr[i].state!=STA_END)
            continue;
        else
            return i; 
    }
    return 0;//0表示没有找到，0一定被使用了，所以不会冲突
}

int new_proc(unsigned int addr, int priority, const char * name)
{
    int i;
    if(!(i=find_proc()))
        return 0;

    int l=0;
    while(*(name+l)!='\0') l++; 
    if(l>20) {
        printf("too long name\n");
        return 0;
    }
    memcpy(proc_arr[i].name, name, l);
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

    proc_arr[i].state = STA_SLEEP;
    proc_arr[i].next = proc_arr[current].next;
    proc_arr[current].next = i;
    proc_arr[i].prev = current;
    proc_arr[proc_arr[i].next].prev = i;

    proc_arr[i].video_mem = VIDEO_MEM;
    return i;
}

void kill_proc(int i)
{
    if(i==0){
        printf("kill error");
        for(;;);
    }
    io_cli();
    int p = proc_arr[i].prev;
    int n = proc_arr[i].next;
    proc_arr[p].next = n;
    proc_arr[n].prev = p;
    proc_arr[i].state = STA_END;
    io_sti();
}

void awaken(int i)
{
    if(i>=MAX_PROCS || i<=0) {
        printf("error awaken");
        for(;;);
    }
    proc_arr[i].state = STA_WAKE;
}

void sleep(int i)
{
    if(i>=MAX_PROCS || i<=0) {
        printf("error awaken");
        for(;;);
    }
    proc_arr[i].state = STA_SLEEP;
}

void switch_proc()
{
    //效率感觉太低了，先不管优化了
    int j = current;
    int t = proc_arr[current].term;
    terminal_table[t]->x = cursor_x;
    terminal_table[t]->y = cursor_y;
    while(1){
        j = proc_arr[j].next;
        if(proc_arr[j].state==STA_WAKE)
            break;
    }
    if(j!=current) {
        current = j;
        time_to_switch = proc_arr[current].priority*10;
        //wait_key();
        video_mem = proc_arr[current].video_mem;
        int term = proc_arr[current].term;
        cursor_x = terminal_table[term]->x;
        cursor_y = terminal_table[term]->y;
        cur_term = term;
        v_move_cursor(cursor_x, cursor_y);
        farjmp(0, proc_arr[j].selector);
    }
    else {
        time_to_switch = proc_arr[current].priority*100;
        //printf("%d", time_to_switch);
        return;
    }
}


void init_lock()
{
    lock_kb.locked = 0;
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
        return 1;
    }
    //printf("%d",lk->locked);

    while(xchg(&lk->locked,1)!=0);
    __sync_synchronize();
    //printf("%d",lk->locked);

    lk->pid = current;
    return 1;
}

void release_lock(struct lock * lk)
{
    if(!holding(lk)){
        printf("release error");
        for(;;);
    }

    __sync_synchronize();
    asm volatile("movl $0, %0" : "+m" (lk->locked) : );
}

void show_proc()
{
    for(int i=0;i<MAX_PROCS;i++){
        if(proc_arr[i].state!=STA_END)
            printf("%d.next = %d .prev = %d  ",i,proc_arr[i].next,proc_arr[i].prev);
    }
}

void show_proc_for_user()
{
    printf("show process information\n");
    for(int i=0;i<MAX_PROCS;i++){
        if(proc_arr[i].state!=STA_END){
            printf("  pid:%d  name:%s state:",i,proc_arr[i].name);
            if(proc_arr[i].state==STA_SLEEP)printf("sleep ");
            else if(proc_arr[i].state==STA_START)printf("start ");
            else if(proc_arr[i].state==STA_WAKE)printf("wake  ");

            printf("priority:%d  \n",proc_arr[i].priority);
        }
    }
}

void exec(int i)
{
    switch_proc();
}

void init_proc()
{
    initFirstProc();
    init_lock();
    // int b,a;
    // if(b = new_proc(main_b, 100))
    //     printf("success b\n");
    // if(a = new_proc(main_a,100))
    //     printf("success a\n");
    // show_proc();

    //for(int i=0;;i++) if(i%100000==0)printf("%d ",proc_arr[current].priority);
    //for(;;);
}