#include "func_def.h"

// args macro for printf
#ifndef PRINTF_ARG
#define PRINTF_ARG

typedef char *va_list;

/* Amount of space required in an argument list for an arg of type TYPE.
   TYPE may alternatively be an expression whose type is used.  */

#define __va_rounded_size(TYPE) \
    (((sizeof(TYPE) + sizeof(int) - 1) / sizeof(int)) * sizeof(int))

#ifndef __sparc__
#define va_start(AP, LASTARG) \
    (AP = ((char *)&(LASTARG) + __va_rounded_size(LASTARG)))
#else
#define va_start(AP, LASTARG) \
    (__builtin_saveregs(),    \
     AP = ((char *)&(LASTARG) + __va_rounded_size(LASTARG)))
#endif

//void va_end (va_list);		/* Defined in gnulib */
//#define va_end(AP)

#define va_arg(AP, TYPE)            \
    (AP += __va_rounded_size(TYPE), \
     *((TYPE *)(AP - __va_rounded_size(TYPE))))

#endif

void printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    char c, *s;

    while (*fmt)
    {
        c = *fmt++;
        if (c != '%')
        {
            v_putchar(c);
            continue;
        }
        c = *fmt++;
        if (c == '\0')
            break;
        switch (c)
        {
        case 'd':
            printnum(va_arg(ap, int), 10, 1);
            break;
        case 'u':
            printnum(va_arg(ap, int), 10, 0);
            break;
        case 'x':
            printnum(va_arg(ap, int), 16, 0);
            break;
        case 's':
            s = va_arg(ap, char *);
            while (*s)
                v_putchar(*s++);
            break;
        case '%':
            v_putchar('%');
        }
    }
    return;
}

void printnum(int num, int base, int sign)
{
    static const char digits[] = "0123456789ABCDEF";
    static char buf[16];

    if (num == 0)
    {
        v_putchar('0');
        return;
    }

    if (sign && num < 0) // Check for sign or unsign
    { 
        v_putchar('-');
        num = -num;
    }

    int cnt = 0;
    while (num)
    {
        buf[cnt++] = digits[num % base];
        num = num / base;
    }
    for (int i = cnt - 1; i >= 0; i--)
    {
        v_putchar(buf[i]);
    }
}