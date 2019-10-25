#ifndef STRUCT_DEF_H
#define STRUCT_DEF_H

struct idt_descriptor
{
    short offset_low;
    short selector;
    char dw_count, access_right;
	short offset_high;
};



#endif
