#ifndef FUNC_DEF_H
#define FUNC_DEF_H

#include "macro_def.h"
#include "struct_def.h"

// implement in sys_head.S
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_load_eflags();
void io_store_eflags(int eflags);
void io_store_idtr(int limit, int addr);
void inthandler21(void);
void inthandler20(void);

// implemented in hardware_init.c
void init_video();
void init_pic();
void init_idt();
void set_idtdesc(struct idt_descriptor *id, int offset, int selector, int ar);
void init_pit(struct timer_queue *tq);

// implement in text_video.c
void v_clear();
void v_roll_screen();
void v_move_cursor(uint x, uint y);
void v_putchar(char ch);
void v_putchar_at(char ch, uint x, uint y, uint color);

// implement in console_io.c
void printc(char c);               // print char
void printi(int i);                // print integer
void printui(uint ui);             // print unsigned integer
void printhex(uint ui);            // print hex of unsigned integer
void prints(const char *s);        // print string
void printn(float n);              // print float
void printf(const char *fmt, ...); // supports %s %c %d %x %u %f

// implement in mem_manage.c
void memcpy(char *dst, const char *src, int count, int size);
void mem_functest(void);

// implement in byte_buffer.c
void init_byte_buffer(struct byte_buffer *buf);
void put_byte_buffer(struct byte_buffer *buf, byte data);
byte get_byte_buffer(struct byte_buffer *buf);

// implemented in time.c
uint find_timer(struct timer_queue *tq);
void timer_free(struct timer *timer);
void timer_init(struct timer *timer, struct byte_buffer *buf, byte data);
void timer_settime(struct timer *timer, uint timeout, struct timer_queue *tq);
void set_timer(struct timer *timer, struct byte_buffer *buf, byte data, uint timeout, struct timer_queue *tq);

#endif