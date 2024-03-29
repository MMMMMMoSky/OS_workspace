#ifndef FUNC_DEF_H
#define FUNC_DEF_H

#include "macro_def.h"
#include "struct_def.h"

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
void inthandler21(void);
void inthandler20(void);
void inthandler2e(void);
void farjmp(int eip, int cs);
void load_tr(int tr);


// implemented in hardware_init.c
void init_video();
void init_pic();
void init_idt();
void set_idtdesc(struct idt_descriptor *id, int offset, int selector, int ar);
void init_pit(struct timer_queue *tq);

// implement in text_video.c
void v_clear();
void v_roll_screen();
void v_move_cursor(uint x, uint y);
void v_backspace();
void v_putchar(char ch);
void v_putchar_at(char ch, uint x, uint y, uint color);
void mem_v_roll_screen();

// implement in console_io.c
void printc(char c);               // print char
void printi(int i);                // print integer
void printui(uint ui);             // print unsigned integer
void printhex(uint ui);            // print hex of unsigned integer
void prints(const char *s);        // print string
void printn(float n);              // print float
void printf(const char *fmt, ...); // supports %s %c %d %x %u %f
void getline(char *line, int max_len);  // read line
byte strcmp(const char *lhs, const char *rhs);  // compare two strings, if equal return 0
byte strncmp(const char *lhs, const char *rhs, uint length); // compare first length char of two strings

// implement in mem_manage.c
void memcpy(byte* dst, const byte* src, uint count);
void *mem_alloc(uint len);
void mem_free(void *obj, uint size);
void mem_functest(void);
void mem_init_all();
void mem_calc(void);
void mem_printmap(void);
void mem_print_bucketdir();
void mem_print_freebucket();


// implement in byte_buffer.c
void init_byte_buffer(struct byte_buffer *buf);
void put_byte_buffer(struct byte_buffer *buf, byte data);
byte get_byte_buffer(struct byte_buffer *buf);

// implemented in time.c
uint find_timer(struct timer_queue *tq);
void timer_free(struct timer *timer);
void timer_init(struct timer *timer, struct byte_buffer *buf, byte data);
void timer_settime(struct timer *timer, uint timeout, struct timer_queue *tq);
void set_timer(struct timer *timer, struct byte_buffer *buf, byte data, uint timeout, struct timer_queue *tq);

// implement in terminal.c
uint start_new_terminal();
void running_term();
void init_terminal_table();
void switch_terminal(uint target);
uint get_new_terminal();
int set_new_terminal(int n);
void store_cur_term_vram();
void load_cur_term_vram();

// implement in builtin_commands.c
void cmd_echo(const char *param);
void cmd_sleep(const char *param);
void cmd_clear(const char *param);
void cmd_num_conv(const char *param);
void cmd_invalid_cmd(const char *param);
void cmd_calc(const char *param);
void cmd_touch(const char *param);
void cmd_mkdir(const char *param);
void cmd_ls();
void cmd_cd(const char *param);
void cmd_rm(const char *param);
void cmd_pwd();
void cmd_cat(const char *param);
void cmd_show(const char * param);
void cmd_term(const char * param);
void cmd_append(const char *param);
void cmd_proc(const char * param);
void cmd_exit(const char * param);

// implement in file.c
void set_string(struct file_directory *fd, const char *name);
void init_file_system();
void remove_file(struct file_directory *p);
void remove_directory(struct file_directory *p);
struct file_directory* create_new_directory(struct file_directory *path, const char *name);
struct file_directory* create_new_file(struct file_directory *path, const char *name);
void file_append_str(struct file_directory* p, const char* str);
void print_file_context(struct file_directory* p);

#endif