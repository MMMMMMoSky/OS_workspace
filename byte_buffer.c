#include "func_def.h"

void init_byte_buffer(struct byte_buffer *buf)
{
    buf->start = 0;
    buf->end = 0;
    buf->length = 0;
}

void put_byte_buffer(struct byte_buffer *buf, byte data)
{
    if (buf->length < BYTE_BUFFER_SIZE)
    {
        buf->end %= BYTE_BUFFER_SIZE;
        buf->data[buf->end++] = data;
        buf->length++;
    }
    // TODO: else {} (when buffer is full)
}

byte get_byte_buffer(struct byte_buffer *buf)
{
    if (buf->length > 0)
    {
        buf->length--;
        buf->start %= BYTE_BUFFER_SIZE;
        return buf->data[buf->start++];
    }
    else
        return 0;
}
