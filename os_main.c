#include "func_def.h"

struct byte_buffer kb_buf;
struct timer timer1;

void main()
{
    io_cli();

    init_video(); 

    init_idt();
    init_pic();

    init_byte_buffer(&kb_buf);
    init_pit(&timer1);
    io_out8(PIC0_IMR, 0xf8);

    io_sti();
	
    // test timer
    printf("Hello world!");

loop:
    __asm__("nop\n\t");
    goto loop;
}
