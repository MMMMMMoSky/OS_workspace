#include "func_def.h"

void main()
{
	//中断
	init_idt();
	init_pic();
	io_cli();
	io_out8(PIC0_IMR, 0xfd);
	io_sti();
    
    init_video();
	
    // test console_io
    uint a = 0x1234;
    printui(a);
    printc('\n');
    printi(-a);
    printc('\n');
    printhex(a);
    char s[15] = "Hello world";
    printc('\n');
    prints(s);

loop:
    __asm__("nop\n\t");
    goto loop;
}
