#ifndef STRUCT_DEF_H
#define STRUCT_DEF_H

struct idt_descriptor
{
    short offset_low, offset_high;
    short selector;
    char dw_count, access_right;
};



#endif