#include "func_def.h"

void main()
{
	//中断
	init_idt();
	init_pic();
	io_cli();
	io_out8(PIC0_IMR, 0xfd);  // init keyboard
	io_sti();
    
    init_video();
	
    // test printf()
    printf("%d %x %s\n", 123, 0x123, "Hello world");
    int a = -1234;
    uint b = 0xabcd;
    char s[] = "Wu Hao";
    printf("%d %x %s\n", a, b, s);

loop:
    __asm__("nop\n\t");
    goto loop;
}
