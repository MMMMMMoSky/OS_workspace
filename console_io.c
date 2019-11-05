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