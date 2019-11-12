#include "func_def.h"
#include "hdreg.h"
#include "proc.h"

struct byte_buffer kb_buf,timer1_buf,timer2_buf,timer3_buf;
struct timer_queue timer_q;
struct timer timer1,timer2,timer3;
char routine[MAX_CONTEXT_BYTE];

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

    // start terminal process 
    start_new_terminal();
    initFirstProc();
    int term_proc = new_proc(running_term, 10);
    if (test_proc == 0) {
        printf("Error: failed to start new process.\n");
        printf("Try reboot or FIIIIIIIIIX BUG.\n");
        goto loop;
    }

    io_sti();

    // jump to terminal process
    farjmp(0, proc_arr[term_proc].selector);

loop:
    __asm__("nop\n\t");
    goto loop;
}
