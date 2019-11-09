#include "func_def.h"
#include "hdreg.h"

struct byte_buffer kb_buf,timer1_buf,timer2_buf,timer3_buf;
struct timer_queue timer_q;
struct timer timer1,timer2,timer3;

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

    io_sti();

    mem_init_all();
    test_hard_disk();

    // start terminal process 
    start_new_terminal();
    running_term();

loop:
    __asm__("nop\n\t");
    goto loop;
}
