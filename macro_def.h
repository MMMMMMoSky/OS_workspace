#ifndef MACRO_DEF_H
#define MACRO_DEF_H

typedef unsigned int uint;
typedef long long int64;
typedef unsigned long long uint64;

// VGA 80x25x16 text mode
#define VIDEO_MEM 0xB8000
#define VIDEO_X_SZ 80
#define VIDEO_Y_SZ 25

// used when setting PIC
#define PIC0_ICW1       0x0020
#define PIC0_OCW2       0x0020
#define PIC0_IMR        0x0021
#define PIC0_ICW2       0x0021
#define PIC0_ICW3       0x0021
#define PIC0_ICW4       0x0021
#define PIC1_ICW1       0x00a0
#define PIC1_OCW2       0x00a0
#define PIC1_IMR        0x00a1
#define PIC1_ICW2       0x00a1
#define PIC1_ICW3       0x00a1
#define PIC1_ICW4       0x00a1

// cli and sti, use like function
#define io_cli() __asm__("cli\n\t")
#define io_sti() __asm__("sti\n\t")

#endif