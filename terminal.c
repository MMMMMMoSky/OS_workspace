#include "func_def.h"
#include "proc.h"

// 想要支持多终端, 需要修改一众 print, 或者再提供额外的接口
// 目前仅仅单终端使用 ---- 即使可以创建新的终端, 也无法多任务

uint term_cnt = 0;                 // terminal count
uint cur_term;                     // current terminal
byte term_vram[MAX_TERMINAL_CNT][VIDEO_MEM_SIZE]; // when terminal goto background, save vram

struct terminal * terminal_table[MAX_TERMINAL_CNT] ;
extern struct lock lock_kb;
extern struct lock lock_video;
extern struct proc_struct_simple proc_arr[MAX_PROCS];
extern int current;
extern unsigned int video_mem;
extern uint cursor_x, cursor_y;

char cmd_buf[1024];  // TODO: current line, inputing, maybe 1024 too small
extern struct file_directory path_root;  // root
extern struct file_directory *path_now;  // file system path now

extern struct lock lock_kb;
extern struct lock lock_video;

void store_cur_term_vram()
{
    memcpy(terminal_table[cur_term]->term_vram, (byte*)VIDEO_MEM, VIDEO_MEM_SIZE);
}

void load_cur_term_vram()
{
    memcpy((byte*)VIDEO_MEM, terminal_table[cur_term]->term_vram, VIDEO_MEM_SIZE);
}

void switch_terminal(uint target)
{
    // if (target >= term_cnt) return;
    // store_cur_term_vram();
    // cur_term = target;
    // load_cur_term_vram();
    if(term_cnt==1){//表示是第一次创建terminal
        cur_term = target;
        load_cur_term_vram();
        // printf("%d", terminal_table[proc_arr[1].term]->x);
        // printf("%d", terminal_table[proc_arr[1].term]->y);
        //printf("first create terminal");
        return;
    }
    //存储旧terminal信息
    store_cur_term_vram();
    // terminal_table[cur_term]->x = cursor_x;
    // terminal_table[cur_term]->y = cursor_y;

    //跟新当前terminal信息
    cur_term = target;
    // cursor_x = terminal_table[target]->x;
    // cursor_y = terminal_table[target]->y;
    // v_move_cursor(cursor_x, cursor_y);
    //..还有cmd的信息不知道应该如何处理
    load_cur_term_vram();
}


void init_terminal_table()
{
    for(int i=0;i<MAX_TERMINAL_CNT;i++){
        terminal_table[i] = mem_alloc(sizeof(struct terminal));
        terminal_table[i]->flag = 0;
        terminal_table[i]->cmd_len = 0;
        terminal_table[i]->term_vram = mem_alloc(4000);
        terminal_table[i]->x = 0;
        terminal_table[i]->y = 0;
    }
    printf("init terminal ok!\n");
    return ;
}

uint get_new_terminal()
{
    int i;
    for(i=1;i<MAX_TERMINAL_CNT;i++){
        if((terminal_table[i])->flag != 1)
            break;
    }
    if(i==16) return 100;
    terminal_table[i]->flag = 1;
    term_cnt ++;
    return i;
}

int set_new_terminal(int n)
{
    if(terminal_table[n]->flag == 1)
    {
        printf("terminal used!\n");
        return 0;
    }
    terminal_table[n]->flag = 1;
    term_cnt ++;
    return 1;
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
    else if(strcmp(cmd_line, "term") == 0) {
        int t_para = param[0]-'0';
        if(t_para==cur_term || strcmp(param, "")==0) return;
        io_cli();
        if(terminal_table[t_para]->flag == 0)//创建新终端
        {
            int newp = new_proc(running_term, 10);
            if(newp==0){
                printf("error on new proc\n");
                for(;;);
            } 
            proc_arr[newp].video_mem = VIDEO_MEM;
            proc_arr[current].video_mem = terminal_table[proc_arr[current].term]->term_vram;
            set_new_terminal(t_para);
            int t;
            t = proc_arr[newp].term = t_para;//get_new_terminal();
            terminal_table[t]->pid =  newp;

            switch_terminal(t);

            release_lock(&lock_kb);
            awaken(newp);
            exec(newp);
            printf("\n");
        }
        else {//切换终端
            proc_arr[t_para].video_mem = VIDEO_MEM;
            proc_arr[cur_term].video_mem = terminal_table[proc_arr[cur_term].term]->term_vram;
            switch_terminal(t_para);
            release_lock(&lock_kb);

            exec(terminal_table[t_para]->pid);
            printf("\n");
        }
    }
    else if(strcmp(cmd_line, "ch") == 0) {
        
    }
    else if(strcmp(cmd_line, "newp") == 0){
        
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
    // if (term_cnt == 0) {
    //     return ;
    // }
    while (1) {
        printf("%d, [tty%u] %s $ ",current, cur_term, path_now->name);
        if(get_lock(&lock_kb));

        getline(cmd_buf, 1024);
        exec_command(cmd_buf);  // parse and execute command
    }
}

