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
    io_out8(PIC0_IMR, 0xf8);  // 打开键盘和定时器中断

    io_sti();
	
    uint last_timer = timer1.count;
    while (1) {
        io_cli();
        if (kb_buf.length) {
            byte data = get_byte_buffer(&kb_buf);
            io_sti();
            printf("Keyboard pressed: %x\n", (uint)data);
            io_cli();
        }
        if (timer1.count % 100 == 0 && last_timer != timer1.count) {
            last_timer = timer1.count;
            io_sti();
            printf("Timer count: %d\n", timer1.count);
        }
        io_sti();
    }

loop:
    __asm__("nop\n\t");
    goto loop;
}
