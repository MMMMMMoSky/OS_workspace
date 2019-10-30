#include "func_def.h"

void init_video()
{
    v_clear();
    v_move_cursor(0, 0);
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
}

void init_idt()
{
    struct idt_descriptor *idt = (struct idt_descriptor *)IDT_ADDR;
    for (int i = 0; i < IDT_INTR_LIMIT; i++)
    {
        set_idtdesc(idt + i, 0, 0, 0);
    }
    set_idtdesc(idt + 0x20, (int)&inthandler20, 0x08, 0x008e);
    set_idtdesc(idt + 0x21, (int)&inthandler21, 0x08, 0x008e);
    io_store_idtr(0x7ff, IDT_ADDR);
}

void set_idtdesc(struct idt_descriptor *id, int offset, int selector, int ar)
{
    id->offset_low = offset & 0xffff;
    id->selector = selector;
    id->dw_count = (ar >> 8) & 0xff;
    id->access_right = ar & 0xff;
    id->offset_high = (offset >> 16) & 0xffff;
}

void init_pit(struct timer_queue *tq)
{
    io_out8(PIT_CTRL, 0x34);
    io_out8(PIT_CNT0, 0x9c);
    io_out8(PIT_CNT0, 0x2e);
    tq->count = 0;
    tq->next = 0xffffffff;
    for (int i = 0; i < TIMER_NUM; i++)
    {
        tq->timer[i].flags = 0;
    }
}

// 以下两个函数在汇编中被调用, 也没有在 func_def.h 中声明

// 键盘中断调用函数
void keyboard_intr()
{
    byte data;
    io_out8(PIC0_OCW2, 0x61);
    data = io_in8(0x0060);
    extern struct byte_buffer kb_buf;
    put_byte_buffer(&kb_buf, data);
}

// 时间中断IRQ0
void handle_IRQ0(void)
{
    io_out8(PIC0_OCW2, 0X60);
    extern struct timer_queue timer_q;
    timer_q.count++;
    if (timer_q.next > timer_q.count)
    {
        return;
    }
    timer_q.next = MAX_TIME;
    for (int i = 0; i < TIMER_NUM; i++)
    {
        if (timer_q.timer[i].flags == TIMER_FLAGS_USING)
        {
            if (timer_q.timer[i].timeout <= timer_q.count)
            {
                timer_q.timer[i].flags = TIMER_FLAGS_ALLOC;
                put_byte_buffer(timer_q.timer[i].buf, timer_q.timer[i].data);
            }
            else
            {
                if (timer_q.next > timer_q.timer[i].timeout)
                {
                    timer_q.next = timer_q.timer[i].timeout;
                }
            }
        }
    }
}
