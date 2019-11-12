#include "func_def.h"

// 想要支持多终端, 需要修改一众 print, 或者再提供额外的接口
// 目前仅仅单终端使用 ---- 即使可以创建新的终端, 也无法多任务

uint term_cnt = 0;                 // terminal count
uint cur_term;                     // current terminal
byte term_vram[MAX_TERMINAL_CNT][VIDEO_MEM_SIZE]; // when terminal goto background, save vram

char cmd_buf[1024];  // TODO: current line, inputing, maybe 1024 too small
extern struct file_directory path_root;  // root
extern struct file_directory *path_now;  // file system path now

extern struct lock lock_kb;
extern struct lock lock_video;

void store_cur_term_vram()
{
    memcpy(term_vram[cur_term], (byte*)VIDEO_MEM, VIDEO_MEM_SIZE);
}

// start a new terminal and make it foreground
// return id of this terminal
uint start_new_terminal()
{
    if (term_cnt > 0) {
        store_cur_term_vram();
    }
    cur_term = term_cnt++;
    // term_vram[cur_term] = (byte*)mem_alloc(VIDEO_MEM_SIZE); // TODO: mem_alloc bug
}

void exec_command(char *cmd_line)
{
    // remove prefix spaces
    while (*cmd_line && *cmd_line == ' ') cmd_line++;
    if (*cmd_line == 0) return;

    // find first space (or \0)
    char *first_space = cmd_line;
    while (*first_space && *first_space != ' ') first_space++;
    // find the start of parameters
    char *param = first_space;
    while (*param && *param == ' ') param++;

    // temporaryily change the first space to 0, for strcmp()
    char old = *first_space;
    *first_space = 0;

    if (strcmp(cmd_line, "echo") == 0) {
        cmd_echo(param);
    }
    else if (strcmp(cmd_line, "clear") == 0) {
        cmd_clear(param);
    }
    else if (strcmp(cmd_line, "num-conv") == 0) {
        cmd_num_conv(param);
    }
    else if (strcmp(cmd_line, "sleep") == 0) {
        cmd_sleep(param);
    }
    else if (strcmp(cmd_line, "calc") == 0) {
        cmd_calc(param);
    }
    else if (strcmp(cmd_line, "touch") == 0) {
        cmd_touch(param);
    }
    else if (strcmp(cmd_line, "mkdir") == 0) {
        cmd_mkdir(param);
    }
    else if (strcmp(cmd_line, "ls") == 0) {
        cmd_ls();
    }
    else if (strcmp(cmd_line, "cd") == 0) {
        cmd_cd(param);
    }
    else if (strcmp(cmd_line, "rm") == 0) {
        cmd_rm(param);
    }
    else if (strcmp(cmd_line, "pwd") == 0) {
        cmd_pwd();
    }
    else if (strcmp(cmd_line, "cat") == 0) {
        cmd_cat(param);
    }
    else if(strcmp(cmd_line, "show") == 0) {
        cmd_show(param);
    }
    // else if (strcmp(cmd_line, "name") == 0) {
    //     cmd_name(param);
    // }
    else {
        cmd_invalid_cmd(param);
    }

    // resume the first space
    *first_space = old;
}

// terminal process; start this function after all os init done
void running_term()
{
    if (term_cnt == 0) {
        return ;
    }
    while (1) {
        printf("[tty%u] %s $ ", cur_term, path_now->name);
        getline(cmd_buf, 1024);
        exec_command(cmd_buf);  // parse and execute command
    }
}

