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

void main(void)
{
    box_fill(0, 0, 100, 100, 0x03);
    loop:
    goto loop;
}
