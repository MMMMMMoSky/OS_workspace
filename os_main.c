#include "func_def.h"
#include "hdreg.h"
#include "proc.h"

struct byte_buffer kb_buf,timer1_buf,timer2_buf,timer3_buf;
struct timer_queue timer_q;
struct timer timer1,timer2,timer3;
char routine[MAX_CONTEXT_BYTE];
extern struct proc_struct_simple proc_arr[MAX_PROCS];
extern unsigned int video_mem, cursor_x, cursor_y;

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
    init_file_system();

    init_proc();
    init_terminal_table();
    
    int term_proc = new_proc(running_term, 10);
    if (term_proc == 0) {
        printf("Error: failed to start new process.\n");
        printf("Try reboot or FIIIIIIIIIX BUG.\n");
        goto loop;
    }

    proc_arr[term_proc].term = get_new_terminal();
    switch_terminal(proc_arr[term_proc].term);
    awaken(term_proc);
    
    io_sti();
loop:
    __asm__("nop\n\t");
    goto loop;
}
