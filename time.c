#include "func_def.h"

uint find_timer(struct timer_queue *tq) //找到某个闲置定时器的标号
{
    for (uint i = 0; i < TIMER_NUM; i++)
    {
        if (tq->timer[i].flags == 0)
        {
            tq->timer[i].flags = TIMER_FLAGS_ALLOC;
            return i;
        }
    }
    return -1;
}

void timer_free(struct timer *timer) //释放某个定时器
{
    timer->flags = 0;
    return;
}

void timer_init(struct timer *timer, struct byte_buffer *buf, byte data) //初始化定时器
{
    timer->buf = buf;
    timer->data = data;
    return;
}

void timer_settime(struct timer *timer, uint timeout,
                   struct timer_queue *tq) //把定时器加入队列
{
    uint num = find_timer(tq);
    tq->timer[num] = *timer;
    tq->timer[num].timeout = timeout + tq->count;
    tq->timer[num].flags = TIMER_FLAGS_USING;
    if (tq->next > tq->timer[num].timeout)
    {
        tq->next = tq->timer[num].timeout;
    }
    return;
}

void set_timer(struct timer *timer, struct byte_buffer *buf,
               byte data, uint timeout, struct timer_queue *tq) //以秒为单位
{
    timeout *= 100;
    init_byte_buffer(buf);             //为定时器设置一个缓冲
    timer_init(timer, buf, data);      //初始化第3个定时器
    timer_settime(timer, timeout, tq); //放入定时器队列,5s
}
