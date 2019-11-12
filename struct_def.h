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
    byte data[BYTE_BUFFER_SIZE]; //32
    uint start, end, length;
};

struct timer
{
    uint flags; //flag=1表示还未运行已经配置状态，2表示正在运行,0表示未使用
    uint timeout;
    struct byte_buffer *buf;
    byte data;
};

struct timer_queue
{
    uint count, next;
    struct timer timer[TIMER_NUM]; //300
};

/*
 *用于内存管理的结构体
*/
//桶结构体，一个桶代表一页内存
struct bucket_desc
{
    void *page;
    struct bucket_desc *next;
    void *freeptr;
    unsigned short refcnt;
    unsigned short bucket_size;
};

struct _bucket_dir
{
    int size;
    struct bucket_desc *chain;
};

//file
struct file_directory
{
    char context[MAX_CONTEXT_BYTE];
    char name[MAX_NAME_BYTE];
    struct file_directory *left;
    struct file_directory *right;
    struct file_directory *father;
    int flag;//1 directory;0 file
};

struct file_directory_point
{
    struct file_directory *fdp;
};

struct terminal{
    int flag;
    int cmd_len;
    char cmd_buf[1024];
    byte * term_vram;
    int x;
    int y;
    int pid;
};

#endif
