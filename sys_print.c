#include<func_def.h>

//io out
#define inb(port) ({\
unsigned char _v; \
__asm__ volatile("inb %%dx, %%al":"=a" (_v):"d" (port)); \
_v; \
    })
#define outb(port, value) \
    __asm__ ("outb %%al, %%dx"::"a" (value), "d" (port));

#define VIDEO_MEM 0xB8000
#define VIDEO_X_SZ 80
#define VIDEO_Y_SZ 25

//args macro for pringtf
#ifndef _STDARG_H
#define _STDARG_H

typedef char *va_list;

/* Amount of space required in an argument list for an arg of type TYPE.
   TYPE may alternatively be an expression whose type is used.  */

#define __va_rounded_size(TYPE)  \
  (((sizeof (TYPE) + sizeof (int) - 1) / sizeof (int)) * sizeof (int))

#ifndef __sparc__
#define va_start(AP, LASTARG) 						\
 (AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
#else
#define va_start(AP, LASTARG) 						\
 (__builtin_saveregs (),						\
  AP = ((char *) &(LASTARG) + __va_rounded_size (LASTARG)))
#endif

//void va_end (va_list);		/* Defined in gnulib */
//#define va_end(AP)

#define va_arg(AP, TYPE)						\
 (AP += __va_rounded_size (TYPE),					\
  *((TYPE *) (AP - __va_rounded_size (TYPE))))

#endif

int video_x;
int video_y;

char * video_buffer = (char *)0xB8000;
struct video_info {
    unsigned int retval;        // Return value
    unsigned int colormode;     // Color bits
    unsigned int feature;       // Feature settings
};


void video_putchar_at(char ch, int x, int y, char attr)
{
	if(x >= 80)
	{	
		x = 80;
	}
	if(y >= 25)
	{	
		y = 25;
	}
	char *video = (char *)0xb8000;
	*(video + (x+y*80)*2) = ch;
	*(video + (x+y*80)*2 + 1) = attr;	
	return;
}

void video_init() {
    //struct video_info *info = 0x9000;

    video_x = 0;
    video_y = 0;
    video_clear();
    update_cursor(video_y, video_x);
}

void video_clear() {
    int i;
    int j;
    video_x = 0;
    video_y = 0;
    for(i = 0; i < VIDEO_X_SZ; i++) {
        for(j = 0; j < VIDEO_Y_SZ; j++) {
           video_putchar_at(' ', i, j, 0x0F); 
        }
    }
    return ;
}

void update_cursor(int row, int col) {
    unsigned int pos = (row * VIDEO_X_SZ) + col;
    // LOW Cursor port to VGA Index Register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    // High Cursor port to VGA Index Register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
    return ;
}


void video_putchar(char ch) {
    if(ch == '\n') {
        video_x = 0;
        video_y++;
    }
    else {
        video_putchar_at(ch, video_x, video_y, 0x0F);
        video_x++;

    }
    if(video_x >= VIDEO_X_SZ) {
        video_x = 0;
        video_y++;
    }
    if(video_y >= VIDEO_Y_SZ) {
        roll_screen();
        video_x = 0;
        video_y = VIDEO_Y_SZ - 1;
    }

    update_cursor(video_y, video_x);
    return ;
}

void roll_screen() {
    int i;
    // Copy line A + 1 to line A
    for(i = 1; i < VIDEO_Y_SZ; i++) {
        memcpy(video_buffer + (i - 1) * 80 * 2, video_buffer + i * 80 * 2, VIDEO_X_SZ, 2*sizeof(char));
    }
    // Clear the last line
    for(i = 0; i < VIDEO_X_SZ; i++) {
        video_putchar_at(' ', i, VIDEO_Y_SZ - 1, 0x0F);
    }
    return ;
}


void memcpy(char *dest, char *src, int count, int size) {
    int i;
    int j;
    for(i = 0; i < count; i++) {
        for(j = 0; j < size; j++) {
            *(dest + i*size + j) = *(src + i*size + j);
        }
    }
    return ;
}

void printf(char *fmt,...)
{
    va_list ap;
    va_start(ap, fmt);

    char c, *s;

    while(*fmt) {
        c = *fmt++;
        if(c != '%') {
            video_putchar(c);
            continue;
        }
        c = *fmt++;
        if(c == '\0')
            break;
        switch(c) {
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
                s = va_arg(ap, char*);
                while(*s)
                    video_putchar(*s++);
                break;
            case '%':
                video_putchar('%');
        }
    }
    return;
}

//在函数中用不了数组, 所以使用全局数组代替一下
char printnum_buf[50]="";
char printnum_digits[] = "0123456789ABCDEF";
int cnt = 0;
void printnum(int num, int base, int sign) {
    int i;
	//虽然可以申明全局变量,但是在外面赋值这个数组没有用,只好在函数内在赋值一次了...
	for(i = 0;i<16;i++)
	{
		if(i < 10)
			printnum_digits[i] = '0'+i;		
		else
			printnum_digits[i] = 'A'+i-10;
	}
    if(sign && num < 0) {       // Check for sign or unsign
        video_putchar('-');
        num = -num;
    }

    if(num == 0) {
        video_putchar('0');
        return ;
    }

    while(num) {
        printnum_buf[cnt++] = printnum_digits[num % base];
        num = num / base;
    }

    for(i = cnt - 1; i >=0; i--) {
        video_putchar(printnum_buf[i]);
    }
    return ;
}
