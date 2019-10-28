#ifndef STRUCT_DEF_H
#define STRUCT_DEF_H

struct idt_descriptor
{
    short offset_low;
    short selector;
    char dw_count, access_right;
    short offset_high;
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


/*
 *用于内存管理的结构体
*/
//桶结构体，一个桶代表一页内存
struct bucket_desc
{
    void                *page;
	struct bucket_desc	*next;
	void			    *freeptr;
	unsigned short		refcnt;
	unsigned short		bucket_size;
};

struct _bucket_dir
{
    int size;
    struct bucket_desc *chain;
};


#endif
