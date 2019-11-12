#include "func_def.h"

void printc(char c)
{
    v_putchar(c);
}

void printi(int i)
{
    uint ui = i;
    if (i < 0) {
        v_putchar('-');
        ui = i - 1 < i ? (uint)-i : (uint)1 << 31;
    }
    printui(ui);
}

void printui(uint ui)
{
    static char digits[20];

    if (ui == 0) {
        v_putchar('0');
        return ;
    }

    digits[19] = 0;
    uint idx = 19, tmp;
    while (ui) {
        digits[--idx] = '0' + (ui % 10);
        ui /= 10;
    }
    prints(digits + idx);
}

void printhex(uint ui)
{
    static char digits[20];
    static const char digit2char[] = "0123456789ABCDEF";

    if (ui == 0) {
        prints("0x0");
        return ;
    }

    digits[19] = 0;
    uint idx = 19;
    while (ui) {
        digits[--idx] = digit2char[ui & 0xf];
        ui >>= 4ull;
    }
    prints("0x");
    prints(digits + idx);
}

void prints(const char * s)
{
    while (*s) {
        v_putchar(*s);
        s++;
    }
}
void printn(float d)
{
    // TODO need to finish
    prints("a float");
}

void printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    while (*fmt) {
        char c = *fmt++;
        if (c != '%') {
            v_putchar(c);
            continue;
        }
        c = *fmt++;
        if (c == '\0') break;
        switch (c) {
            case 'c': v_putchar(va_arg(ap, char)); break;
            case 'd': printi(va_arg(ap, int)); break;
            case 'u': printui(va_arg(ap, uint)); break;
            case 'x': printhex(va_arg(ap, uint)); break;
            case 'f': printn(va_arg(ap, float)); break;
            case 's': prints(va_arg(ap, const char *)); break;
            case '%': v_putchar('%'); break;
            default: break;
        }
    }
}

byte strcmp(const char *lhs, const char *rhs)
{
    while ((*lhs) == (*rhs)) {
        if ((*lhs) == 0) return 0;
        lhs++; rhs++;
    }
    return 1;
}

byte strncmp(const char *lhs, const char *rhs, uint length)
{
    for (uint i = 0; i < length; i++) {
        if (lhs[i] == 0 || rhs[i] == 0 || lhs[i] != rhs[i]) {
            return 1;
        }
    }
    return 0;
}

extern struct byte_buffer kb_buf;

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

byte ctrl_down;  // whether control is pressed, default 0, not pressed
byte shift_down; // whether shift is pressed, default 0, not pressed
byte cap_lock;   // whether capital lock is opened, default 0, closed

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

void getline(char *line, int max_len)
{
    int len = 0;
    while (len < max_len - 1) {  // -1 (reserve a place for '\0')
        if (kb_buf.length == 0) continue;
        io_cli();
        byte data = get_byte_buffer(&kb_buf);  // 可以优化, 一次取多个
        io_sti();

        // determine if data is control key: shift, enter, cap lock, backspace
        if (data == BACKSPACE_DOWN) {
            if (len == 0) continue;
            len--;
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
        line[len++] = c;
        v_putchar(c);
    }
    line[len] = 0;
    v_putchar('\n');
}