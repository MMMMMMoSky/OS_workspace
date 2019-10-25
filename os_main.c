#include "func_def.h"

//暂时用于键盘中断调用函数
void print(void)
{	
	int data;
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(0x0060);
	v_putchar('a');
}

void main()
{
	//中断
	init_idt();
	init_pic();
	set_idtdesc(0x0026f800 + 0x21, 0, 0x08, 0x008e);
	io_cli();
	io_out8(PIC0_IMR, 0xfd);
	io_sti();
    
    init_video();
	
	for(;;)
	{;}
    // test console_io
    uint a = 0x1234;
    printui(a);
    printc('\n');
    printi(-a);
    printc('\n');
    printhex(a);

loop:
    __asm__("nop\n\t");
    goto loop;
}
