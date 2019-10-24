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


// implemented in hardware_init.c
void init_video();
void init_pic();
void init_idt();
void set_idtdesc(struct idt_descriptor *id, int offset, int selector, int ar);


// implement in text_video.c
void v_clear();
void v_roll_screen();
void v_move_cursor(uint x, uint y);
void v_putchar(char ch);
void v_putchar_at(char ch, uint x, uint y, uint color);

// implement in console_io.c
void printf(const char *fmt,...);
void printnum(int num, int base, int sign);

// implement in mem_manage.c
void memcpy(char *dst, const char *src, int count, int size);

#endif