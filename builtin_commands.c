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