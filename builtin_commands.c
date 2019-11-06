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