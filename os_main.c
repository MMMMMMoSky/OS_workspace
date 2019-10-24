#include "func_def.h"

void main()
{
    init_idt();
    init_pic();
    io_cli();
loop:
    __asm__("nop\n\t");
    goto loop;
}

void init_pic()
{
    io_out8(PIC0_IMR, 0xff);
    io_out8(PIC1_IMR, 0xff);

    io_out8(PIC0_ICW1, 0x11);
    io_out8(PIC0_ICW2, 0x20);
    io_out8(PIC0_ICW3, 1 << 2);
    io_out8(PIC0_ICW4, 0x01);

    io_out8(PIC1_ICW1, 0x11);
    io_out8(PIC1_ICW2, 0x28);
    io_out8(PIC1_ICW3, 2);
    io_out8(PIC1_ICW4, 0x01);

    io_out8(PIC0_IMR, 0xfb);
    io_out8(PIC1_IMR, 0xff);

    return;
}

void set_idtdesc(struct idt_descriptor *id, int offset, int selector, int ar)
{
    id->offset_low = offset & 0xffff;
    id->selector = selector;
    id->dw_count = (ar >> 8) & 0xff;
    id->access_right = ar & 0xff;
    id->offset_high = (offset >> 16) & 0xffff;
}

void init_idt()
{
    struct idt_descriptor *idt = (struct idt_descriptor *)0x0026f800;
    for (int i = 0; i < 256; i++) {
        set_idtdesc(idt + i, 0, 0, 0);
    }
    io_store_idtr(0x7ff, 0x0026f800);
}