#include "func_def.h"

//用于测试的全局变量
int  x = 200;

void main(void)
{
	box_fill(0, 0, 100, 100, x);
    for (int i = 0; i < 256; i++) {
        //box_fill(0, 0, 100, 100, i);
        nop(10000000);
        // asm_hlt();
    }
    loop:
    asm_hlt();
    goto loop;
}

void nop(int repeat)
{
    for (int k = 0; k < repeat; k++) {
        __asm__("nop\n\t");
    }
}

void box_fill(int bx, int by, int sx, int sy, char c)
{
    for (int i = 0; i < sx; i++) {
        for (int j = 0; j < sy; j++) {
            int x = bx + i;
            int y = by + j;
            *((char*)0xa0000 + x * 320 + y) = c;
        }
    }
}
