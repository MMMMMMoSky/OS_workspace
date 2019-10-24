#include<func_def.h>

#define inb(port) ({\
unsigned char _v; \
__asm__ volatile("inb %%dx, %%al":"=a" (_v):"d" (port)); \
_v; \
    })

#define outb(port, value) \
    __asm__ ("outb %%al, %%dx"::"a" (value), "d" (port));
#define VIDEO_MEM 0xB8000
#define VIDEO_X_SZ 80
#define VIDEO_Y_SZ 25
int video_x;
int video_y;

char * video_buffer = (char *)0xB8000;
struct video_info {
    unsigned int retval;        // Return value
    unsigned int colormode;     // Color bits
    unsigned int feature;       // Feature settings
};


void video_putchar_at(char ch, int x, int y, char attr)
{
	if(x >= 80)
	{	
		x = 80;
	}
	if(y >= 25)
	{	
		y = 25;
	}
	char *video = (char *)0xb8000;
	*(video + (x+y*80)*2) = ch;
	*(video + (x+y*80)*2 + 1) = attr;	
	return;
}

void video_init() {
    //struct video_info *info = 0x9000;

    video_x = 0;
    video_y = 0;
    video_clear();
    update_cursor(video_y, video_x);
}

void video_clear() {
    int i;
    int j;
    video_x = 0;
    video_y = 0;
    for(i = 0; i < VIDEO_X_SZ; i++) {
        for(j = 0; j < VIDEO_Y_SZ; j++) {
           video_putchar_at(' ', i, j, 0x0F); 
        }
    }
    return ;
}

void update_cursor(int row, int col) {
    unsigned int pos = (row * VIDEO_X_SZ) + col;
    // LOW Cursor port to VGA Index Register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    // High Cursor port to VGA Index Register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
    return ;
}


void video_putchar(char ch) {
    if(ch == '\n') {
        video_x = 0;
        video_y++;
    }
    else {
        video_putchar_at(ch, video_x, video_y, 0x0F);
        video_x++;

    }
    if(video_x >= VIDEO_X_SZ) {
        video_x = 0;
        video_y++;
    }
    if(video_y >= VIDEO_Y_SZ) {
        roll_screen();
        video_x = 0;
        video_y = VIDEO_Y_SZ - 1;
    }

    update_cursor(video_y, video_x);
    return ;
}

void roll_screen() {
    int i;
    // Copy line A + 1 to line A
    for(i = 1; i < VIDEO_Y_SZ; i++) {
        memcpy(video_buffer + (i - 1) * 80 * 2, video_buffer + i * 80 * 2, VIDEO_X_SZ, 2*sizeof(char));
    }
    // Clear the last line
    for(i = 0; i < VIDEO_X_SZ; i++) {
        video_putchar_at(' ', i, VIDEO_Y_SZ - 1, 0x0F);
    }
    return ;
}


void memcpy(char *dest, char *src, int count, int size) {
    int i;
    int j;
    for(i = 0; i < count; i++) {
        for(j = 0; j < size; j++) {
            *(dest + i*size + j) = *(src + i*size + j);
        }
    }
    return ;
}


