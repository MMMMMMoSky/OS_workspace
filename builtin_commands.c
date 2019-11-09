#include "func_def.h"

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
                prints("Error: time must be an interger\n");
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
                prints("Error: sleep time1 -d time2\n");
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
     if(*cmd<cmd_end) *cmd+=1;
     else error = 0;
    do{
        if(E == 1){
        if(*cmd<cmd_end) *cmd+=1;
        else error = 0;
        }
        E=1;
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
	        else if(calc_param[*cmd-1].calc_sym == 6)
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
                        printf("test 310\n");
                         return -1;
                         }
	            }
	        else{
                printf("test 315\n");
		         return -1;
            }
        }while(error!=0 && calc_param[*cmd-1].calc_sym!=3 && calc_param[*cmd-1].calc_sym!=4);
    }while(error!=0 && calc_param[*cmd-1].calc_sym!=1 && calc_param[*cmd-1].calc_sym!=2);
   // printf("  this is a test sentence  at 315");
    return error;
}

//calculate expression
void cmd_calc_calc(calc_word* calc_param,int*calc_answer,const uint cmd_end)
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
                      || calc_param[param_cmd].calc_sym == 6)){
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
     stack_prep=deep_stack=houzhui_cmd=0;
     ////////////////////////computing   suffix  expressions
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
     //printf("  this is a test sentence  445\n");
     //mem_free(calc_stack,cmd_end);mem_free(houzhui,cmd_end);
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
    //get  effective  length 
    /*while(param[cmd] != '\0'){
        if(param[cmd] != ' ') cmd_end++;
        cmd++;
    }
    cmd=0;*/
    //generate  a valid string
    //  动态分配内存的函数出了问题。先使用静态数组   11.08
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
             case '（':
				calc_param[cmd].calc_sym=6;
				break;
			case ')':
				calc_param[cmd].calc_sym=7;
				break;
                case '）':
				calc_param[cmd].calc_sym=7;
                break;
			}
            cmd+=1;calc_cmd+=1;
		}
		// char is figure 
		else if(calc_char[calc_cmd] >='0' && calc_char[calc_cmd] <='9')
		{
            //printf("test\n");
            calc_param[cmd].calc_sym=5;
			while(calc_char[calc_cmd] >='0' && calc_char[calc_cmd] <='9')
			{
				calc_param[cmd].figure=10*calc_param[cmd].figure+(int*)(calc_char[calc_cmd]-'0');
				calc_cmd+=1;
			}
            cmd+=1;
		}
		// other char
		else  
		{
            //printf("  this is a test sentence at  535\n");
			return -1;
		}
	}
    cmd_end = cmd;
    cmd=0;
    while(cmd<cmd_end-1){
        if(calc_param[cmd].calc_sym==4 && calc_param[cmd+1].calc_sym==5&&calc_param[cmd+1].figure==0)
      { 
       //printf("  this is a test sentence at  543\n");
       return -1;
       }
        //printf("%d    ",calc_param[cmd].figure);
        cmd+=1;
    }
    int _cmd=0;
    //////////////////////////////////grammatical   analysis//////////////////////////////
    calc_error = cmd_calc_prog(calc_param,&_cmd,cmd_end);
    if(calc_error == 0) cmd_calc_calc(&calc_param,calc_answer,cmd_end);
    //free memory
    /*cmd=cmd_end=0;
    while(param[cmd] != 0){
        if(param[cmd] != ' ') cmd_end++;
        cmd++;
    }
    mem_free(calc_char,cmd_end+1);mem_free(calc_param,cmd_end);*/
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