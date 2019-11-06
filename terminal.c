#include "func_def.h"

// 想要支持多终端, 需要修改一众 print, 或者再提供额外的接口
// 目前仅仅单终端使用 ---- 即使可以创建新的终端, 也无法多任务
// 目前对键盘的处理不支持长按连续输入
// 仅仅判断 keyDown 编码, 而忽略 keyUp 编码

#define ENTER_DOWN 0x1c
#define ENTER_UP 0x9c
#define LSHIFT_DOWN 0x2a
#define LSHIFT_UP 0xaa
#define RSHIFT_DOWN 0x36
#define RSHIFT_UP 0xb6
#define CAP_LOCK 0x3a   // press cap lock, 0x3a 0xba 0x3a 0xba
#define BACKSPACE_DOWN 0x0e  
#define BACKSPACE_UP 0x8e  

uint term_cnt = 0;                 // terminal count
uint cur_term;                     // current terminal
byte term_vram[MAX_TERMINAL_CNT][VIDEO_MEM_SIZE]; // when terminal goto background, save vram

uint cmd_len;
char cmd_buf[1024];  // TODO: current line, inputing, maybe 1024 too small
extern struct byte_buffer kb_buf;

byte ctrl_down;  // whether control is pressed, default 0, not pressed
byte shift_down; // whether shift is pressed, default 0, not pressed
byte cap_lock;   // whether capital lock is opened, default 0, closed

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

char kb_decode(byte data)
{
    // key up
    if (0x82 <= data) return 0;

    // 1234567890-= 0x2~0xd
    static const char *num_line = "1234567890-=";
    static const char *num_line_shift = "!@#$%^&*()_+";
    if (0x02 <= data && data <= 0x0d) {
        return shift_down ? num_line_shift[data - 0x02] : num_line[data - 0x02];
    }

    // qwertyuiop[] 0x10~0x1b
    static const char *qwe_line = "qwertyuiop[]";
    static const char *qwe_line_shift = "QWERTYUIOP{}";
    if (0x10 <= data && data <= 0x1b) {
        uint idx = data - 0x10; 
        if (idx < 10 && cap_lock) {
            return shift_down ? qwe_line[idx] : qwe_line_shift[idx];
        }
        return shift_down ? qwe_line_shift[idx] : qwe_line[idx];
    }

    // asdfghjkl;'` 0x1e~0x29
    static const char *asd_line = "asdfghjkl;'`";
    static const char *asd_line_shift = "ASDFGHJKL:\"~";
    if (0x1e <= data && data <= 0x29) {
        uint idx = data - 0x1e; 
        if (idx < 9 && cap_lock) {
            return shift_down ? asd_line[idx] : asd_line_shift[idx];
        }
        return shift_down ? asd_line_shift[idx] : asd_line[idx];
    }

    // \zxcvbnm,./ 0x2b~0x35
    static const char *zxc_line = "\\zxcvbnm,./";
    static const char *zxc_line_shift = "|ZXCVBNM<>?";
    if (0x2b <= data && data <= 0x35) {
        uint idx = data - 0x2b; 
        if (1 <= idx && idx < 8 && cap_lock) {
            return shift_down ? zxc_line[idx] : zxc_line_shift[idx];
        }
        return shift_down ? zxc_line_shift[idx] : zxc_line[idx];
    }

    // space 0x39
    if (data == 0x39) {
        return ' ';
    }

    return 0;
}

void exec_command(char *cmd_line)
{
    uint cmd = 0;
    while (cmd_line[cmd] != ' ' && cmd_line[cmd] != 0) {
        cmd++;
    }
    char old = cmd_line[cmd];
    cmd_line[cmd] = 0;

    if (strcmp(cmd_line, "echo") == 0) {
        cmd_echo(cmd_line + cmd + 1);
    }
    else if (strcmp(cmd_line, "clear") == 0) {
        cmd_clear(cmd_line + cmd + 1);
    }
    else if (strcmp(cmd_line, "num-conv") == 0) {
        cmd_num_conv(cmd_line + cmd + 1);
    }
    // else if (strcmp(cmd_line, "name") == 0) {
    //     cmd_name(cmd_line + cmd + 1);
    // }
    else {
        cmd_invalid_cmd(cmd_line + cmd + 1);
    }

    cmd_line[cmd] = old;
}

// terminal process; start this function after all os init done
void running_term()
{
    if (term_cnt == 0) {
        return ;
    }
    while (1) {
        printf("tty%u >> ", cur_term);
        while (1) {
            if (kb_buf.length == 0) continue;
            io_cli();
            byte data = get_byte_buffer(&kb_buf);  // 可以优化, 一次取多个
            io_sti();

            // determine if data is control key: shift, enter, cap lock, backspace
            if (data == BACKSPACE_DOWN) {
                if (cmd_len == 0) continue;
                cmd_len--;
                v_backspace();
                continue;
            }
            if (data == CAP_LOCK) {
                cap_lock = (cap_lock + 1) & 3;  // & 3 (% 4)
                continue;
            }
            if (data == LSHIFT_DOWN || data == RSHIFT_DOWN) {
                shift_down++;
                continue;
            }
            if (data == LSHIFT_UP || data == RSHIFT_UP) {
                shift_down--;
                continue;
            }
            if (data == ENTER_DOWN) {
                break;  // parse command
            }
            if (data == ENTER_UP || data == BACKSPACE_UP) continue;

            char c = kb_decode(data);
            if (c == 0) continue;
            cmd_buf[cmd_len++] = c;
            v_putchar(c);
        }
        v_putchar('\n');
        cmd_buf[cmd_len] = 0;
        exec_command(cmd_buf);  // parse and execute command
        cmd_len = 0;
    }
}

