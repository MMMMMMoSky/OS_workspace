#ifndef STRUCT_DEF_H
#define STRUCT_DEF_H

struct idt_descriptor
{
    short offset_low;
    short offset_high;
    short selector;
    char dw_count, access_right;
};

struct byte_buffer
{
    byte data[BYTE_BUFFER_SIZE];
    uint start, end, length;
};

struct timer 
{
    uint count;
    uint timeout;
    struct byte_buffer *buf;
    byte data;
};

#endif
