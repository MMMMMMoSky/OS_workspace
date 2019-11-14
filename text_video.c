#include "func_def.h"
#include "proc.h"

unsigned int video_mem = VIDEO_MEM;
uint cursor_x, cursor_y;
extern struct proc_struct_simple proc_arr[MAX_PROCS];
extern int current;
extern uint cur_term;  
extern struct terminal * terminal_table[MAX_TERMINAL_CNT] ;
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
    if(proc_arr[current].term==cur_term)
        v_move_cursor(cursor_x, cursor_y);
}

void v_putchar(char ch)
{
    if (ch == '\n')
    {
        cursor_x = 0;
        cursor_y++;
        terminal_table[cur_term]->line++;
        int l = terminal_table[cur_term]->line;
        memcpy(
                (byte*)(terminal_table[cur_term]->term_vram + (l - 1) * VIDEO_X_SZ * 2), 
                (byte*)(video_mem + (cursor_y-1) * VIDEO_X_SZ * 2), 
                VIDEO_X_SZ * 2
            );
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
        terminal_table[cur_term]->line++;
        int l = terminal_table[cur_term]->line;
        memcpy(
                (byte*)(terminal_table[cur_term]->term_vram + (l - 1) * VIDEO_X_SZ * 2), 
                (byte*)(video_mem + (cursor_y-1) * VIDEO_X_SZ * 2), 
                VIDEO_X_SZ * 2
            );
    }
    if (cursor_y >= VIDEO_Y_SZ)
    {
        v_roll_screen();
        cursor_x = 0;
        cursor_y = VIDEO_Y_SZ - 1;
    }

    if(terminal_table[cur_term]->line >= ALL_Y_SZ){
        mem_v_roll_screen();
        terminal_table[cur_term]->line = ALL_Y_SZ - 1;
    }
    if(proc_arr[current].term==cur_term)
        v_move_cursor(cursor_x, cursor_y);
}

void v_putchar_at(char ch, uint x, uint y, uint color)
{
    x = x < VIDEO_X_SZ ? x : VIDEO_X_SZ - 1;
    y = y < VIDEO_Y_SZ ? y : VIDEO_Y_SZ - 1;
    char *pos = (char *)(video_mem + (x + y * VIDEO_X_SZ) * 2);
    *pos = ch;
    *(pos + 1) = color & 0xff;
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


extern struct terminal * terminal_table[MAX_TERMINAL_CNT] ;
void v_roll_screen()
{
    //其实此时基本不会输出
    if(video_mem!=VIDEO_MEM){
        //...
    }
    else
    {
        //移动存储中的屏幕
        int len = terminal_table[cur_term]->cmd_len;
        char * v = terminal_table[cur_term]->term_vram;
        terminal_table[cur_term]->cmd_len = len+1;
        // 移动真的屏幕
        // Copy line A + 1 to line A
        for (int i = 1; i < VIDEO_Y_SZ; i++)
        {
            memcpy(
                (byte*)(video_mem + (i - 1) * VIDEO_X_SZ * 2), 
                (byte*)(video_mem + i * VIDEO_X_SZ * 2), 
                VIDEO_X_SZ * 2
            );
        }
        // Clear the last line
        for (int i = 0; i < VIDEO_X_SZ; i++)
        {
            v_putchar_at(0, i, VIDEO_Y_SZ - 1, 0x0f);
        }   
    }
}

void mem_v_roll_screen()
{
    terminal_table[cur_term]->cmd_len--;
    for(int i = 1;i<ALL_Y_SZ;i++){
        memcpy(
                (byte*)(terminal_table[cur_term]->term_vram + (i - 1) * VIDEO_X_SZ * 2), 
                (byte*)(terminal_table[cur_term]->term_vram + i * VIDEO_X_SZ * 2), 
                VIDEO_X_SZ * 2
            );
    }
    for (int i = 0; i < VIDEO_X_SZ; i++)
    {
        char *pos = (char *)(terminal_table[cur_term]->term_vram + (i + (ALL_Y_SZ-1) * VIDEO_X_SZ) * 2);
        *pos = 0;
        *(pos + 1) = 0x0f;
        // *(terminal_table[cur_term]->term_vram + (ALL_Y_SZ - 1) + 2 * i) = 0;
        // *(terminal_table[cur_term]->term_vram + (ALL_Y_SZ - 1) + 2 * i + 1) = 0x0f ; 
    }   
}