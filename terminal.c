#include "func_def.h"

// 想要支持多终端, 需要修改一众 print, 或者再提供额外的接口
// 目前仅仅单终端使用 ---- 即使可以创建新的终端, 也无法多任务

uint term_cnt = 0;                 // terminal count
uint cur_term;                     // current terminal
byte term_vram[MAX_TERMINAL_CNT][VIDEO_MEM_SIZE]; // when terminal goto background, save vram

uint cmd_len;
char cmd_buf[256];  // TODO: current line, inputing, maybe 256 too small
extern struct byte_buffer kb_buf;

void store_cur_term_vram()
{
    memcpy(term_vram[cur_term], (byte*)VIDEO_MEM, VIDEO_MEM_SIZE);
}

// start a new terminal and make it foreground
// return id of this terminal
uint start_new_terminal()
{
    if (term_cnt > 0) {
        store_cur_term_vram();
    }
    cmd_len = 0;
    cur_term = term_cnt++;
    // term_vram[cur_term] = (byte*)mem_alloc(VIDEO_MEM_SIZE); // TODO: mem_alloc bug
}

// terminal process; start this function after all os init done
void running_term()
{
    if (term_cnt == 0) {
        return ;
    }
    while (1) {
        printf("T%u >> ", cur_term);
        while (1) {
            if (kb_buf.length == 0) continue;
            io_cli();
            byte data = get_byte_buffer(&kb_buf);  // 可以优化, 一次取多个
            io_sti();
            // char c = kb_decode(data);  // TODO: kb_decode
            if (data == 0x1c) break;
            if (data == 0x9c) continue; // enter: press 1c, release 9c
            cmd_buf[cmd_len++] = data;
            printf("%x ", data);
        }
        // TODO parse_command
        printc('\n');
    }
}

