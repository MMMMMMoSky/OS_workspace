#include "func_def.h"

uint cursor_x, cursor_y;

void v_backspace()
{
    if (cursor_x == 0) {
        if (cursor_y == 0) return;
        cursor_x = VIDEO_X_SZ - 1;
        cursor_y--;
    }
    else {
        cursor_x--;
    }
    v_putchar_at(0, cursor_x, cursor_y, 0x0f);
}

void v_putchar(char ch)
{
    if (ch == '\n')
    {
        cursor_x = 0;
        cursor_y++;
    }
    else
    {
        v_putchar_at(ch, cursor_x, cursor_y, 0x0f);
        cursor_x++;
    }
    if (cursor_x >= VIDEO_X_SZ)
    {
        cursor_x = 0;
        cursor_y++;
    }
    if (cursor_y >= VIDEO_Y_SZ)
    {
        v_roll_screen();
        cursor_x = 0;
        cursor_y = VIDEO_Y_SZ - 1;
    }

    v_move_cursor(cursor_x, cursor_y);
}

void v_putchar_at(char ch, uint x, uint y, uint color)
{
    x = x < VIDEO_X_SZ ? x : VIDEO_X_SZ - 1;
    y = y < VIDEO_Y_SZ ? y : VIDEO_Y_SZ - 1;
    char *pos = (char *)(VIDEO_MEM + (x + y * VIDEO_X_SZ) * 2);
    *pos = ch;
    *(pos + 1) = color & 0xff;
    v_move_cursor(x, y);
}

void v_clear()
{
    cursor_x = cursor_y = 0;
    for (uint i = 0; i < VIDEO_X_SZ; i++)
    {
        for (uint j = 0; j < VIDEO_Y_SZ; j++)
        {
            v_putchar_at(0, i, j, 0x0f);
        }
    }
}

void v_move_cursor(uint x, uint y)
{
    uint pos = (y * VIDEO_X_SZ) + x;
    // LOW Cursor port to VGA Index Register
    io_out8(0x3d4, 0xf);
    io_out8(0x3d5, pos & 0xff);
    // High Cursor port to VGA Index Register
    io_out8(0x3d4, 0x0e);
    io_out8(0x3d5, (pos >> 8) & 0xff);
}

void v_roll_screen()
{
    // Copy line A + 1 to line A
    for (int i = 1; i < VIDEO_Y_SZ; i++)
    {
        memcpy(
            (byte*)(VIDEO_MEM + (i - 1) * VIDEO_X_SZ * 2), 
            (byte*)(VIDEO_MEM + i * VIDEO_X_SZ * 2), 
            VIDEO_X_SZ * 2
        );
    }
    // Clear the last line
    for (int i = 0; i < VIDEO_X_SZ; i++)
    {
        v_putchar_at(0, i, VIDEO_Y_SZ - 1, 0x0f);
    }
}