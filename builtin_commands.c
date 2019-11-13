#include "func_def.h"
#include "hdreg.h"
#include "proc.h"

extern struct file_directory path_root;  // root
extern struct file_directory *term_path[MAX_TERMINAL_CNT]; 

struct terminal * terminal_table[MAX_TERMINAL_CNT];
extern struct lock lock_kb;
extern struct lock lock_video;
extern struct proc_struct_simple proc_arr[MAX_PROCS];
extern int current;
extern unsigned int video_mem;
extern uint cursor_x, cursor_y;
extern uint cur_term;

void cmd_echo(const char *param)
{
    printf("%s\n", param);
}

void cmd_invalid_cmd(const char *param)
{
    printf("Error: invalid command\n");
}

void cmd_clear(const char *param)
{
    v_clear();
}

void cmd_num_conv_help()
{
    prints(
        "\n"
        "num-conv\n"
        "  convert number base\n"
        "Usage: num-conv [options] number\n"
        "Options:\n"
        "  -f, -from    specify origin base of number (default 10)\n"
        "  -t, -to      specify target base (default 10)\n"
        "  -h, -help    print this page\n"
        "number: a positive integer\n"
        "Notice: the base must be in the range of [2, 16]\n"
        "\n"
    );
}

uint parse_digit(char c)
{
    if ('0' <= c && c <= '9') {
        return c - '0';
    }
    if ('a' <= c && c <= 'f') {
        return c - 'a' + 10;
    }
    if ('A' <= c && c <= 'F') {
        return c - 'A' + 10;
    }
    return 0xffffffffu;
}

void convert_num_base(uint number, uint target_base, char *result)
{
    if (number == 0) {
        result[0] = '0';
        result[1] = 0;
        return ;
    }

    uint len = 0;
    while (number) {
        uint digit = number % target_base;
        result[len++] = digit + (digit < 10 ? '0' : 'A' - 10);
        number /= target_base;
    }
    result[len] = 0;

    for (uint l = 0, r = len - 1; l < r; l++, r--) {
        char tmp = result[l];
        result[l] = result[r];
        result[r] = tmp;
    }
}

// taboo: I just use goto when syntax error occurred
void cmd_num_conv(const char *param)
{
    const char *p = param;
    uint from = 0, to = 0;

    // 1. parse options
    while (1) {
        while (*p && *p != '-') p++;  // find '-'
        if (*p == 0) break;
        p++;
        if (strncmp(p, "h", 1) == 0 || strncmp(p, "help", 4) == 0) {
            cmd_num_conv_help();
            return ;
        }
        else if (strncmp(p, "f ", 2) == 0 || strncmp(p, "from ", 5) == 0) {
            if (from) {
                prints("Error: -f can be used at most once!\n");
                goto cmd_num_conv_bad_syntax;
            }
            // parse -f
            while (*p != ' ') p++;  // skip from
            while (*p == ' ') p++;  // skip spaces after -from
            while (*p && *p != ' ') {
                if (*p < '0' || *p > '9') {
                    goto cmd_num_conv_base_out_of_range;
                }
                from = from * 10 + (*p) - '0';
                p++;
            }
            if (from < 2 || from > 16) {
                goto cmd_num_conv_base_out_of_range;
            }
        }
        else if (strncmp(p, "t ", 2) == 0 || strncmp(p, "to ", 3) == 0) {
            if (to) {
                prints("Error: -t can be used at most once!\n");
                goto cmd_num_conv_bad_syntax;
            }
            // parse -t
            while (*p != ' ') p++;
            while (*p == ' ') p++;
            while (*p && *p != ' ') {
                if (*p < '0' || *p > '9') {
                    goto cmd_num_conv_base_out_of_range;
                }
                to = to * 10 + (*p) - '0';
                p++;
            }
            if (to < 2 || to > 16) {
                goto cmd_num_conv_base_out_of_range;
            }
        }
        else {
            prints("Error: invalid option\n");
            goto cmd_num_conv_bad_syntax;
        }
    }
    // default base
    if (from == 0) from = 10;
    if (to == 0) to = 10;

    // 2. parse number
    p = param;
    while (*p) {
        while (*p && *p == ' ') p++;
        if (*p == '-') {  // skip an option
            while (*p && *p != ' ') p++;  // skip -from/-f/-to/-t
            while (*p && *p == ' ') p++;  // skip spaces after -from
            while (*p && *p != ' ') p++;  // skip base number
        }
        else break;
    }
    if (*p == 0) {
        prints("Error: please input number\n");
        goto cmd_num_conv_bad_syntax;
    }
    // start calc number
    uint number = 0;
    while (*p && *p != ' ') {
        uint digit = parse_digit(*p);
        if (digit >= from) {
            printf("Error: invalid number in %u-base.\n", from);
            goto cmd_num_conv_bad_syntax;
        }
        number = number * from + digit;
        p++;
    }

    // 3. convert number
    char res[64];
    convert_num_base(number, to, res);
    prints(res);
    printc('\n');
    return ;

cmd_num_conv_base_out_of_range:
    prints("Error: base must be in range of [2, 16]\n");
    goto cmd_num_conv_bad_syntax;

cmd_num_conv_bad_syntax:
    prints("Try 'num-conv -h' for more information\n");
}

void cmd_sleep(const char *param)
{
    extern struct timer_queue timer_q;
    struct timer timer_sleep, timer_sleep_gap;
    struct byte_buffer sleep_buffer, sleep_buffer_gap;
    int i = 0;
    int flag = 0;
    int flag2 = 0;
    if (strcmp(param, " ") == 0 || strcmp(param, "") == 0)
    {
        prints("Error: sleep time1 -d time2 and time must be an interger\n");
        return;
    }
    if (strcmp(param, "-h") == 0)
    {
        prints("\nsleep\n  using timer implement sleep\nUsage: sleep time or sleep time1 [options] time2\nOPtions:\n -d, -duration    output the time that has sleep at regular intervals\n -h, -help        print this page\ntime: a positive integar\n\n");
        return;
    }
    while (param[i] != '\0')
    {
        if (param[i] == '-' && param[i + 1] == 'd')
        {
            flag = 1;
        }
        i++;
    }
    int length = i;
    if (flag == 0)
    {
        int sum_time = 0;
        for (int i = 0; i < length && flag2 != -1; i++)
        {
            if (param[i] > 47 && param[i] < 59)
            {
                sum_time = sum_time * 10 + param[i] - 48;
            }
            else
            {
                flag2 = -1;
                prints("Error: sleep time1 -d time2 and time must be an interger\n");
                break;
            }
        }

        if (flag2 != -1)
        {
            set_timer(&timer_sleep, &sleep_buffer, '1', sum_time / 1000, &timer_q);
            for (;;)
            {
                if (sleep_buffer.length != 0)
                {
                    prints("sleep end.\n");
                    break;
                }
            }
        }
    }
    if (flag == 1)
    {
        int sum_time = 0;
        int gap_time = 0;
        int sequence = 1;
        for (int i = 0; i < length && flag2 != -1; i++)
        {
            if (sequence == 1 && param[i] > 47 && param[i] < 59)
            {
                sum_time = sum_time * 10 + param[i] - 48;
            }
            else if (sequence == 1 && param[i++] == ' ' && param[i++] == '-' && param[i++] == 'd' && param[i] == ' ')
            {
                sequence = 2;
                continue;
            }
            else if (sequence == 2 && param[i] > 47 && param[i] < 59)
            {
                gap_time = gap_time * 10 + param[i] - 48;
            }
            else
            {
                flag2 = -1;
                prints("Error: sleep time1 -d time2 and time must be an interger\n");
                break;
            }
        }
        if (flag2 != -1)
        {
            set_timer(&timer_sleep_gap, &sleep_buffer_gap, '1', gap_time / 1000, &timer_q);
            int num = 1;
            for (;;)
            {
                if (num == sum_time / gap_time + 1)
                {
                    prints("sleep end.\n");
                    break;
                }
                if (sleep_buffer_gap.length != 0)
                {
                    printf("sleep %dms\n", gap_time * (num++));
                    timer_free(&timer_sleep_gap);
                    set_timer(&timer_sleep_gap, &sleep_buffer_gap, '1', gap_time / 1000, &timer_q);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////
//cmd_calc
    typedef struct{
         int figure;int calc_sym;
     }calc_word;

int cmd_calc_prog(calc_word* calc_param,int* cmd, int cmd_end)
{
    uint E=0;uint T=0;uint error=-1;
    do{
        if(E == 1){
        if(*cmd<cmd_end) *cmd+=1;
        else error = 0;
        }
        E=1;
        T=0;
        do{
            if(T ==1){
            if(*cmd<cmd_end) *cmd+=1;
             else error = 0;
            }
            T=1;
            if (calc_param[*cmd-1].calc_sym == 5){
                 if(*cmd<cmd_end) *cmd+=1;
                 else error = 0;
            }
	        else if(calc_param[*cmd-1].calc_sym == 6 )
	            {
                   if(*cmd<cmd_end) *cmd+=1;
                    else error = 0;
		            cmd_calc_prog(calc_param,cmd,cmd_end);
		            if (calc_param[*cmd-1].calc_sym == 7){
                         if(*cmd<cmd_end) *cmd+=1;
                         else error = 0;
                    }
		            else
			             {
                         return -1;
                         }
	            }
	        else{
		         return -1;
            }
        }while( calc_param[*cmd-1].calc_sym==3 || calc_param[*cmd-1].calc_sym==4 );
    }while( calc_param[*cmd-1].calc_sym==1 || calc_param[*cmd-1].calc_sym==2 );
    //printf("back\n");
    return error;
}

//calculate expression
void cmd_calc_calc(calc_word* calc_param,int*calc_answer,uint cmd_end)
{
     typedef union{
         char sym;
         int number;
     }cmd_stack;
     //cmd_stack* calc_stack=(cmd_stack*)mem_alloc(cmd_end*1);
     cmd_stack calc_stack[1024]={0};
     uint deep_stack=0;uint stack_now=0;uint stack_prep=0;
     uint param_cmd = 0;uint houzhui_cmd = 0;
     //cmd_stack* houzhui=(cmd_stack*)mem_alloc(cmd_end*1);
     cmd_stack houzhui[1024]={0};

     //////////////////////////generate suffix expressions
     printf("start to generate suffix expressions\n");
     while(param_cmd<cmd_end){
         //figure
         if(calc_param[param_cmd].calc_sym == 5){
             houzhui[houzhui_cmd].number=calc_param[param_cmd].figure;
             param_cmd+=1;houzhui_cmd+=1;
         }
         else{
             //the current stack is empty
             if(deep_stack == 0){
                 deep_stack++;
                 if(calc_param[param_cmd].calc_sym == 1) calc_stack[0].sym = '+';
                 if(calc_param[param_cmd].calc_sym == 2) calc_stack[0].sym = '-';
                 if(calc_param[param_cmd].calc_sym == 3) calc_stack[0].sym = '*';
                 if(calc_param[param_cmd].calc_sym == 4) calc_stack[0].sym = '/';
                 if(calc_param[param_cmd].calc_sym == 6) calc_stack[0].sym = '(';
                 if(calc_param[param_cmd].calc_sym == 7) calc_stack[0].sym = ')';
                 param_cmd++;
             }
              //the current stack is not empty
             else{
                 //right parenthesis
                 if(calc_param[param_cmd].calc_sym == 7){
                     stack_now = 0;
                     while(stack_now<deep_stack && calc_stack[stack_now].sym!='('){
                         houzhui[houzhui_cmd].sym=calc_stack[stack_now].sym;
                         houzhui_cmd++;stack_now++;
                     }
                     while(stack_prep<(deep_stack-stack_now-1)){
                         calc_stack[stack_prep].sym = calc_stack[stack_prep+stack_now+1].sym;
                         stack_prep++;
                         }
                         deep_stack-=(stack_now+1);stack_now=stack_prep=0;
                         param_cmd++;
                 }
                 else{ 
                    //char is not an open parenthesis       &&         priority is not greater than the top of the stack
                    //&&  top of stack is not left parenthesis
                     while(!(calc_stack[0].sym == '(' || 
                     ((calc_stack[0].sym == '+'||calc_stack[0].sym == '-')
                     &&(calc_param[param_cmd].calc_sym == 3 ||calc_param[param_cmd].calc_sym == 4))
                      || calc_param[param_cmd].calc_sym == 6) && deep_stack>0 ){
                          houzhui[houzhui_cmd].sym=calc_stack[0].sym;
                          houzhui_cmd++;
                          deep_stack-=1;
                          while(stack_prep<deep_stack){
                              calc_stack[stack_prep].sym=calc_stack[stack_prep+1].sym;
                              stack_prep++;
                          }
                          stack_prep=0;
                      }
                      //priority is greater than the top of the stack  OR   char is an open parenthesis
                      stack_prep = deep_stack;deep_stack++;
                      while(stack_prep>0){
                           calc_stack[stack_prep] = calc_stack[stack_prep-1];
                           stack_prep--;
                      }
                       if(calc_param[param_cmd].calc_sym == 1) calc_stack[0].sym = '+';
                       if(calc_param[param_cmd].calc_sym == 2) calc_stack[0].sym = '-';
                       if(calc_param[param_cmd].calc_sym == 3) calc_stack[0].sym = '*';
                       if(calc_param[param_cmd].calc_sym == 4) calc_stack[0].sym = '/';
                       if(calc_param[param_cmd].calc_sym == 6) calc_stack[0].sym = '(';
                       if(calc_param[param_cmd].calc_sym == 7) calc_stack[0].sym = ')';
                       param_cmd++;
                 }
             }
         }
     }
     while(stack_prep<deep_stack){
         houzhui[houzhui_cmd].sym=calc_stack[stack_prep].sym;
         stack_prep++;houzhui_cmd++;
     }
    stack_now= stack_prep=deep_stack=houzhui_cmd=0;
     ////////////////////////computing   suffix  expressions
     printf("start to computing   suffix  expressions\n");
     while(stack_prep<cmd_end){
         if(calc_param[stack_prep].calc_sym ==6 || calc_param[stack_prep].calc_sym ==7)
         stack_now++;
         stack_prep++;
         }
         cmd_end=cmd_end-stack_now;
         stack_prep=stack_now=0;

     while(houzhui_cmd<cmd_end){
         //figure
         if(houzhui[houzhui_cmd].sym!='+' && houzhui[houzhui_cmd].sym!='-' 
         && houzhui[houzhui_cmd].sym!='*' && houzhui[houzhui_cmd].sym!='/'){
             stack_prep=deep_stack;deep_stack++;
             while(stack_prep>0){
                 calc_stack[stack_prep].number=calc_stack[stack_prep-1].number;
                 stack_prep--;
             }
             calc_stack[0].number=houzhui[houzhui_cmd].number;
             houzhui_cmd++;stack_prep=0;
         }
         else{
         // char is '+'
         if(houzhui[houzhui_cmd].sym == '+')
             calc_stack[0].number=calc_stack[1].number+calc_stack[0].number;
         //char is '-'
         else if(houzhui[houzhui_cmd].sym == '-')
              calc_stack[0].number=calc_stack[1].number-calc_stack[0].number;
         //char is '*'
         else if(houzhui[houzhui_cmd].sym == '*')
              calc_stack[0].number=calc_stack[1].number*calc_stack[0].number;
         //char is '/'
         else if(houzhui[houzhui_cmd].sym == '/')
              calc_stack[0].number=calc_stack[1].number/calc_stack[0].number;
              stack_now=1;deep_stack--;
              while(stack_now<deep_stack){
                  calc_stack[stack_now].number=calc_stack[stack_now+1].number;
                  stack_now++;
              }
              stack_now=0;houzhui_cmd++;
         }
     }
     *calc_answer=calc_stack[0].number;
}

int cmd_calc_get_error(const char*param,int*calc_answer)  //generate criterion
{
    int calc_error = 0;
    uint cmd = 0;
    uint calc_cmd=0;
    uint cmd_end = 0;
    //parameter-free
    if(param[cmd] == 0)
        return -2;
    //parameter is  '-h'
    else if(param[cmd] == '-' && param[cmd+1] == 'h'){
        cmd_end=cmd+2;
        while(param[cmd_end] == ' ') cmd_end++;
        if(param[cmd_end] == 0) return 1;
        return -1;
    }
    //generate  a valid string
    //char* calc_char = (char*)mem_alloc((cmd_end+1)*1);
    //calc_char[cmd_end] = 0;
    char calc_char [1024]={0};
    while(param[cmd] != '\0'){
        if(param[cmd]!=' '){
            calc_char[cmd_end] = param[cmd];
            cmd_end++;
            }
        cmd+=1;
    }
    calc_char[cmd_end]='\0';
    calc_cmd =0;
    cmd = 0;
    //////////////////////////generate  words//////////////////////////////
    //calc_word* calc_param = (calc_word*)mem_alloc(cmd_end*2);
    calc_word calc_param[1024]={0};
     while( calc_char[calc_cmd]!='\0')
	{
        calc_param[cmd].figure=calc_param[cmd].calc_sym=0;
		// char  is   +   or  -    or   *   or    /   or    (   or    )
		if(calc_char[calc_cmd]  == '(' || calc_char[calc_cmd] == ')' || 
        calc_char[calc_cmd] == '+'  || calc_char[calc_cmd] == '-' || 
        calc_char[calc_cmd]  == '*' || calc_char[calc_cmd] == '/'||
        calc_char[calc_cmd]  == '（' || calc_char[calc_cmd] == '）')
		{
            //printf("test\n");
			switch (calc_char[calc_cmd])
			{
			case '+':
				calc_param[cmd].calc_sym=1;
				break;
			case '-':
				calc_param[cmd].calc_sym=2;
				break;
			case '*':
				calc_param[cmd].calc_sym=3;
				break;
			case '/':
				calc_param[cmd].calc_sym=4;
				break;
			case '(':
				calc_param[cmd].calc_sym=6;
                break;
			case ')':
				calc_param[cmd].calc_sym=7;
				break;
			}
            cmd+=1;calc_cmd+=1;
		}
		// char is figure 
		else if(calc_char[calc_cmd] >='0' && calc_char[calc_cmd] <='9')
		{
            calc_param[cmd].calc_sym=5;
			while(calc_char[calc_cmd] >='0' && calc_char[calc_cmd] <='9')
			{
				calc_param[cmd].figure=10*calc_param[cmd].figure+(calc_char[calc_cmd]-'0');
                calc_cmd+=1;
			}
            cmd+=1;
		}
		// other char
		else  
		{
			return -1;
		}
	}
    cmd_end = cmd;
    cmd=0;
    while(cmd<cmd_end-1){
        if(calc_param[cmd].calc_sym==4 && calc_param[cmd+1].calc_sym==5&&calc_param[cmd+1].figure==0)
      { 
       return -1;
       }
        cmd+=1;
    }
    int _cmd=1;
    //////////////////////////////////grammatical   analysis//////////////////////////////
    calc_error = cmd_calc_prog(calc_param,&_cmd,cmd_end);
    if(calc_error == 0) cmd_calc_calc(&calc_param,calc_answer,cmd_end);
    //free memory
    return calc_error;     
}

void cmd_calc(const char*param)  //computational  expression
{
    //criterion
    int calc_error=0; 
    int calc_answer=0;
    calc_error = cmd_calc_get_error(param,&calc_answer);
    //parameter-free
     if(calc_error == -2)  printf("Error: Parameters are missing\n");
     //invalid parameter
    if(calc_error == -1)  printf("Error: invalid expression. Try 'calc -h' for more info\n");
    //valid parameter
    if(calc_error == 0) printf("the answer is: %d  \n",calc_answer);
    //see the help
    if(calc_error == 1){
       printf(" calc  supports  integer(≤32767)  expression\n");
       printf(" supported  operations:  +   -   *   /   \n");
       printf("format:      calc  [expression]\n");
    }
}
////////////////////////////////////////////////////////////////////////////////////////

// 移动一步, 从 now 移动一步 dir
// 返回移动之后的指针, now 不会被改变
// 不存在则返回 0
struct file_directory* parse_path_step(const char *dir, uint length, struct file_directory *now) {
    // 1. .
    if (length == 1 && strncmp(dir, ".", 1) == 0) {
        return now;
    }
    // 2. ..
    if (length == 2 && strncmp(dir, "..", 2) == 0) {
        if (now->father) {
            return now->father;
        }
        else {
            return now;
        }
    }
    // 3. child folder
    for (struct file_directory* p = now->left; p; p = p->right) {
        if (strncmp(dir, p->name, length) == 0 && p->name[length] == 0) {  // p->name[length] == 0
            return p;
        }
    }
    return 0;  // do not exist
}

// 解析路径 (相对路径/绝对路径)
// path: 路径
// now: 当前路径
// return: 解析成功则返回成功解析的路径节点, 解析失败则返回0 (NULL)
struct file_directory* parse_path(const char *path, struct file_directory *now) {
    // strip spaces
    uint length = 0;
    while (*path == ' ') path++;
    while (path[length]) length++;

    // special judge root dir
    if (length == 1 && *path == '/') {
        return &path_root;
    }

    // strip postfix spaces
    while (length && path[length - 1] == '/') length--;

    uint start, end;
    if (*path == '/') {  // absolute path
        now = &path_root;
        start = end = 1;
    }
    else {  // relative path
        start = end = 0;
    }
    for (; end < length; start = ++end) {
        while (end < length && path[end] != '/') end++;
        // [start, end), a single path
        now = parse_path_step(path + start, end - start, now);
        if (now == 0) return 0;
    }
    return now;
}

// 分离路径和文件(夹)名
// path: 路径 (绝对路径/相对路径)
// now: 当前路径
// name: 从 path 中解析出的文件(夹)名
// return: 成功则返回文件(夹)所在的路径, 解析失败则返回0
// example: /home/sky/tmp.txt 分离得到 /home/sky 和 tmp.txt
struct file_directory* parse_path_and_name(const char *path, 
    struct file_directory *now, char *name) {

    // strip spaces
    uint length = 0;
    while (*path == ' ') path++;
    while (path[length]) length++;

    // copy path
    memcpy(name, path, length);
    name[length] = 0;

    // find last slash (extract file name)
    uint slash = length;
    for (uint i = 0; i < length; i++) {
        if (name[i] == '/') slash = i;
    }

    struct file_directory *p = 0;
    if (slash == length) { // no slash, path is just a filename
        return now;
    }

    name[slash] = 0;
    now = parse_path(name, now);
    for (int i = slash + 1; i <= length; i++) {
        name[i - slash - 1] = name[i];
    }

    return now;
}

void cmd_cd(const char *param)
{
    struct file_directory *res = parse_path(param, term_path[cur_term]);
    if (res == 0) {
        prints("Error: invalid path\n");
    }
    else if (res->start_block >= 0) {
        prints("Error: can not enter a file\n");
    }
    else {
        term_path[cur_term] = res;
    }
}

void cmd_touch_mkdir(const char *param, int flag)
{
    char name[1024];
    struct file_directory *path = parse_path_and_name(param, term_path[cur_term], name);

    if (path == 0) {
        printf("Error: invalid path.\n");
        return;
    }

    int length = 0;
    if (name[0] == '.') {
        printf("Error: name can not start with '.'\n");
        return ;
    }
    while (name[length]) {
        if (!(('0' <= name[length] && name[length] <= '9') ||
            ('a' <= name[length] && name[length] <= 'z') ||
            ('A' <= name[length] && name[length] <= 'Z') ||
            name[length] == '.' || name[length] == '_')) {
            printf("Error: name should only contain 0-9, a-z, A-Z, '_', '.'\n");
            return ;
        }
        length++;
    }
    if (length > MAX_NAME_BYTE || length <= 0) {
        printf("Error: length of name should be in range of [1, %d]\n", MAX_NAME_BYTE);
        return;
    }

    if (parse_path_step(name, length, path)) {
        printf("Error: file/directory already exist.\n");
        return;
    }

    if (flag) create_new_directory(path, name);
    else create_new_file(path, name);
}

void cmd_mkdir(const char *param)
{
    cmd_touch_mkdir(param, 1);
}

// 创建文件
void cmd_touch(const char *param)
{
    cmd_touch_mkdir(param, 0);
}

void cmd_ls()
{
    if (term_path[cur_term]->start_block >= 0) {
        prints("Error: now you are in a file but not a directory\n");
        return;
    }
    for (struct file_directory *p = term_path[cur_term]->left; p; p = p->right) {
        printf("%s  %s\n", (p->start_block < 0 ? "DIR " : "FILE"), p->name);
    }
}

void cmd_pwd()
{
    // special judge if nowdf is root
    if (term_path[cur_term]->father == 0) {
        prints("/\n");
        return;
    }

    int length = 0;
    char *res = (char*)mem_alloc(4096);
    struct file_directory *p = term_path[cur_term];
    for (; p->father; p = p->father) {
        // 将每一级节点名称倒着复制到 res 中
        int pname = 0;
        while (p->name[pname]) pname++;
        while (pname > 0) res[length++] = p->name[--pname];
        res[length++] = '/';
    }
    for (int i = length - 1; i >= 0; i--) {
        printc(res[i]);
    }
    printc('\n');

    mem_free(res, 4096);
}

void cmd_rm_help()
{
    prints(
        "\n"
        "rm\n"
        "  remove file/directory in file system\n"
        "Usage: rm [option] path\n"
        "Options:\n"
        "  -r, -recursion   remove directory\n"
        "  -h, -help        print this page\n"
        "path: absolute or relative path of file/directory\n"
        "Notice: 1. 'rm -r /' will remove all file in root directory\n"
        "           but will not remove the root directory itself\n"
        "        2. if current path is in the folder to remove, it will be reset to root\n"
        "\n"
    ); 
}

void cmd_rm(const char *param)
{
    // 1. parse option
    int length = 0;
    char path[1024];
    byte option_r = 0;
    while (*param) {
        while (*param == ' ') param++;
        if (strncmp(param, "-h", 2) == 0 || strncmp(param, "-help", 5) == 0) {
            cmd_rm_help();
            return;
        }
        if (strncmp(param, "-r", 2) == 0 && (param[2] == 0 || param[2] == ' ')) {
            option_r = 1;
            param += 2;
            continue;
        }
        if (strncmp(param, "-recursion", 10) == 0 && (param[10] == 0 || param[10] == ' ')) {
            option_r = 1;
            param += 10;
            continue;
        }
        while (*param && *param != ' ') path[length++] = *param++;
        path[length] = 0;
    }

    // 2. try to find file/directory
    struct file_directory *p;
    p = parse_path(path, term_path[cur_term]);
    if (p == 0) {
        printf("Error: invalid path.\n");
        return ;
    }
    if ((p->start_block < 0) != option_r) {
        printf("Error: can not remove a directory without -r or remove a file with -r\n");
        printf("Try 'rm -h' for more information\n");
        return;
    }

    // 3. call file system's remove function
    if (option_r) remove_directory(p);
    else remove_file(p);
}

void cmd_cat(const char *param)
{
    struct file_directory *file = parse_path(param, term_path[cur_term]);
    if (file == 0) {
        printf("Error: invalid file path\n");
    }
    print_file_context(file);
}

void cmd_show(const char * param)
{   
    char *ch;
    for(ch=param;;ch++){
        if(*ch=='\0')
            goto label_test;
        if(*ch==' ')
            break;
    }
    *ch = '\0';
    ch++;
    while(*ch!='\0'){
        if(*ch!=' '){
            printf("invalid option!\n");
            return;
        }
        ch++;
    }

label_test:
    if(!strcmp(param, "hdd")){
        show_hard_info();
    }
    else if(!strcmp(param, "mem")){
        printf("mem information:\n");
        mem_printmap();
    }
    else if(!strcmp(param, "pg")){
        printf("page_dir information:\n");
        mem_calc();
    }
    else if(!strcmp(param, "ps")){
        show_proc_for_user();
    }
    else if(!strcmp(param, "-h")){
        printf("command: show\n"
               "summary: show some information of systemn\n"
               "parameter: [option]\n"
               "option:\n"
               " page (show page_dir information)\n"
               " hdd (show hard disk information)\n"
               " mem (show memory information)\n"
        );
    }
    else {
        printf("invalid options\n");
    }
}


void cmd_term(const char * param)
{
    int t_para;
    if(param[0]=='\0'){
        return;
    }
    if(param[1]=='\0' || param[1]==' '){
        for(char *ch=&param[1];*ch!='\0';ch++){
            if(*ch!=' '){
                printf("error options\n");
                return;
            }
        }
        if(param[0]<'0' || param[0] >'9'){
            printf("error options\n");
            return;
        }
        t_para = param[0]-'0';
    }
    else if(param[2]=='\0'){
        for(char *ch=&param[2];*ch!='\0';ch++){
            if(*ch!=' '){
                printf("error options\n");
                return;
            }
        }
        if(param[0]<'0' || param[0] >'9' || param[1] <'0' || param[1] >'9'){
            printf("error options\n");
            return;
        }
        t_para = (param[0]-'0')*10 + (param[1]-'0');
    }
    else {
        return;
    }
    if(t_para==cur_term) return;
    if(t_para > 15){
        printf("term num <= 15\n");
        return;
    }
    if(terminal_table[t_para]->flag == 0)//创建新终端
    {
        char name[] = "term-i ";
        name[5] = '0'+t_para%10;
        if(t_para>=10) name[6] = '0'+t_para/10;
        //printf("new terminal");
        int newp = new_proc(running_term, 10, name);
        if(newp==0){
            printf("error on new proc\n");
            for(;;);
        } 
        proc_arr[newp].video_mem = VIDEO_MEM;
        proc_arr[current].video_mem = terminal_table[proc_arr[current].term]->term_vram;
        set_new_terminal(t_para);
        proc_arr[newp].term = t_para;//get_new_terminal();
        terminal_table[t_para]->pid =  newp;

        release_lock(&lock_kb);
        switch_terminal(t_para);

        sleep(current);
        awaken(newp);
        exec(newp);
    }
    else {//切换终端
    //printf("change terminal");
    io_cli();
        proc_arr[terminal_table[t_para]->pid].video_mem = VIDEO_MEM;
        proc_arr[current].video_mem = terminal_table[proc_arr[current].term]->term_vram;
        
        release_lock(&lock_kb);
        switch_terminal(t_para);

    io_sti();
        sleep(current);
        awaken(terminal_table[t_para]->pid);
        exec(terminal_table[t_para]->pid);
        //printf("bbbbbbbbbb\n");
    }

}

void test_proc()
{
    for(int i=0;i<10;i++){
        printf("a");
    }
    for(;;);
}

void cmd_proc(const char * param)
{
    if(1){
        char name[] = "proce";
        int newp = new_proc(test_proc, 10,name);
        if(newp==0){
            printf("error on new proc\n");
            for(;;);
        }
        proc_arr[newp].video_mem = VIDEO_MEM;
        proc_arr[newp].term = cur_term;
        awaken(newp);
    }
}

void cmd_append_help()
{
    prints(
        "\n"
        "append\n"
        "  append a line to the end of a file\n"
        "Usage: append [option] path context\n"
        "Options:\n"
        "  -h, -help        print this page\n"
        "path: absolute or relative path of a file\n"
        "Notice: if there are more than 1 space\n"
        "        bewteen path and context\n"
        "        they will be writen into file\n"
        "\n"
    ); 
}

// 向文件末尾添加一行内容
void cmd_append(const char *param)
{
    while (*param == ' ') param++;
    if (strncmp(param, "-h", 2) == 0 || strncmp(param, "-help", 5) == 0) {
        cmd_append_help();
        return ;
    }

    int length = 0;
    char path[1024];
    const char *context = param;
    while (*context != ' ') path[length++] = *context++;
    context++;

    struct file_directory *p = parse_path(path, term_path[cur_term]);
    if (p == 0 || p->start_block < 0) {
        printf("Error: invalid file path\n");
        printf("Try 'append -h' for more information\n");
    }
    else {
        file_append_str(p, context);
        file_append_str(p, "\n");  // TODO: 效率略低, 可优化, 直接改 context
        uint i = 1;
        while (*context) context++, i++;
        printf("%u bytes writen to %s\n", i, p->name);
    }
}