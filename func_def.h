#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

// cli and sti
#define io_cli() __asm__("cli\n\t")
#define io_sti() __asm__("sti\n\t")

struct idt_descriptor
{
    short offset_low, selector;
    char dw_count, access_right;
    short offset_high;
};

// implement in os_main.c
void init_pic();
void init_idt();
void set_idtdesc(struct idt_descriptor *id, int offset, int selector, int ar);

// implement in sys_head.S
int io_in8(int port);
int io_in16(int port);
int io_in32(int port);
void io_out8(int port, int data);
void io_out16(int port, int data);
void io_out32(int port, int data);
int io_load_eflags();
void io_store_eflags(int eflags);
void io_store_idtr(int limit, int addr);

//implement in sys_print.c
void video_putchar_at(char ch, int x, int y, char attr);
void video_putchar(char ch);
void update_cursor(int row, int col);
void video_init();
void video_clear();
void roll_screen();
void memcpy(char *dest, char *src, int count, int size);
void printnum(int num, int base, int sign);
void printf(char *fmt,...);
