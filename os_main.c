#include "func_def.h"

void main()
{
    init_idt();
    init_pic();
    io_cli();
    init_video();

    int a = 0x1234;
    printf("%d %d\n", a, a);
    printf("Hello world\n");

loop:
    __asm__("nop\n\t");
    goto loop;
}
