#include "func_def.h"

void main()
{
    init_idt();
    init_pic();
    io_cli();
    init_video();

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
