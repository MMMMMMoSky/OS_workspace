#include "func_def.h"

void memcpy(char *dst, const char *src, int count, int size)
{
    for (int i = 0; i < count; i++)
    {
        for (int j = 0; j < size; j++)
        {
            *(dst + i * size + j) = *(src + i * size + j);
        }
    }
}