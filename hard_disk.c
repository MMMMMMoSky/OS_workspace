#include "func_def.h"

struct hd_info_struct
{
    int head, sect, cyl, wpcom, lzone, ctl;
};

struct hd_info_struct hd_info;

void sys_setup(void *hard_disk)
{
    hd_info.cyl = *(unsigned short *)hard_disk;
    hd_info.head = *(unsigned char *)(2 + hard_disk);
    hd_info.wpcom = *(unsigned short *)(5 + hard_disk);
    hd_info.ctl = *(unsigned char *)(8 + hard_disk);
    hd_info.lzone = *(unsigned short *)(12 + hard_disk);
    hd_info.sect = *(unsigned char *)(14 + hard_disk);
}

void show_hard_info()
{
    printf("cylinder: %d\n", hd_info.cyl);
    printf("head: %d\n", hd_info.head);
    printf("wpcom: %d\n", hd_info.wpcom);
    printf("ctl: %d\n", hd_info.ctl);
    printf("lzone: %d\n", hd_info.lzone);
    printf("sect: %d\n", hd_info.sect);
}

void test_hard_disk()
{
    void *hard_disk = (void *)0x5f700;
    sys_setup(hard_disk);
    show_hard_info();
}