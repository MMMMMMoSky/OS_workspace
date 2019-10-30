#include "func_def.h"

struct byte_buffer kb_buf,timer1_buf,timer2_buf,timer3_buf;
struct timer_queue timer_q;
struct timer timer1,timer2,timer3;

void main()
{
    
    io_cli();

    init_video(); 

    init_idt();
    init_pic();

    init_byte_buffer(&kb_buf);
    init_pit(&timer_q);
    io_out8(PIC0_IMR, 0xf8);  // 打开键盘和定时器中断

    io_sti();
    //定时器组测试代码
    byte data=1;
    set_timer(&timer1, &timer1_buf, data, 3, &timer_q);//设置第1个定时器
    set_timer(&timer2, &timer2_buf, data, 10, &timer_q);//设置第2个定时器
    set_timer(&timer3, &timer3_buf, data, 5, &timer_q);//设置第3个定时器
    int a[3]={0,0,0};
	//mem_functest();

    while (1) {
        //if(timer_q.count%100==0)
        //{
            //printf("%d",timer_q.count);
        //}
        io_cli();
        if (kb_buf.length) {
            byte data = get_byte_buffer(&kb_buf);
            io_sti();
            printf("Keyboard pressed: %x\n", (uint)data);
            io_cli();
        }
        //test************timer1-3
        if (timer1.buf->data[0] == 1&&a[0] == 0) {
            io_sti();
            a[0]=1;
            printf("Timer1 get ready\n");
        }
        if (timer2.buf->data[0] == 1&&a[1] == 0) {
            io_sti();
            a[1]=1;
            printf("Timer2 get ready\n");
        }
        if (timer3.buf->data[0] == 1&&a[2] == 0) {
            io_sti();
            a[2]=1;
            printf("Timer3 get ready\n");
 
        }
        io_sti();
    }

loop:
    __asm__("nop\n\t");
    goto loop;
}
