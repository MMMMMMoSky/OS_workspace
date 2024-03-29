#include "func_def.h"
#include "hdreg.h"
#include "proc.h"

struct byte_buffer kb_buf,timer1_buf,timer2_buf,timer3_buf;
struct timer_queue timer_q;
struct timer timer1,timer2,timer3;
char routine[MAX_CONTEXT_BYTE];
extern struct proc_struct_simple proc_arr[MAX_PROCS];
extern uint video_mem, cursor_x, cursor_y;
extern struct terminal * terminal_table[MAX_TERMINAL_CNT] ;

void main()  // bochs address: 0x106
{
    // some initialization
    io_cli();

    init_video(); 
    init_idt();
    init_pic();
    init_byte_buffer(&kb_buf);
    init_pit(&timer_q);
    io_out8(PIC0_IMR, 0xf8);  // 打开键盘和定时器中断
    mem_init_all();
    init_hard_disk();
    video_mem = VIDEO_MEM;  // 修复写了终端翻页之后 init_file_system() 无法正常输入输出的问题
    init_file_system();

    init_proc();
    init_terminal_table();
    
    int term_proc = new_proc((uint)running_term, 10, "term-1");
    if (term_proc == 0) {
        printf("Error: failed to start new process.\n");
        printf("Try reboot or FIIIIIIIIIX BUG.\n");
        goto loop;
    }

    proc_arr[term_proc].term = 1;
    set_new_terminal(1);    
    switch_terminal(proc_arr[term_proc].term);
    terminal_table[1]->pid = term_proc;

    awaken(term_proc);
    
    io_sti();
loop:
    __asm__("nop\n\t");
    goto loop;
}
