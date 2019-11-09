#include "func_def.h"
#include "hdreg.h"
#include "fs.h"

#define outb_p(value,port) \
__asm__ ("outb %%al,%%dx\n" \
		"\tjmp 1f\n" \
		"1:\tjmp 1f\n" \
		"1:"::"a" (value),"d" (port))

#define inb_p(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al\n" \
	"\tjmp 1f\n" \
	"1:\tjmp 1f\n" \
	"1:":"=a" (_v):"d" (port)); \
_v; \
})
#define outb(value,port) \
__asm__ ("outb %%al,%%dx"::"a" (value),"d" (port))


#define inb(port) ({ \
unsigned char _v; \
__asm__ volatile ("inb %%dx,%%al":"=a" (_v):"d" (port)); \
_v; \
})
#define port_read(port,buf,nr) \
__asm__("cld;rep;insw"::"d" (port),"D" (buf),"c" (nr):)

#define port_write(port,buf,nr) \
__asm__("cld;rep;outsw"::"d" (port),"S" (buf),"c" (nr):)
#define nop() __asm__ ("nop"::)

#define CMOS_READ(addr) ({ \
outb_p(0x80|addr,0x70); \
inb_p(0x71); \
})


void (*do_hd)(void);

//硬盘参数信息
struct hd_info_struct {
	int head,sect,cyl,wpcom,lzone,ctl;
};
struct hd_info_struct hd_info;

//请求队列
struct request {
    int dev;
	int cmd;		/* READ or WRITE */
	int errors;
	unsigned long sector;
	unsigned long nr_sectors;
	char * buffer;
	//struct task_struct * waiting;
	struct buffer_head * bh;
	struct request * next;
};
struct request * CURRENT;

//硬盘设备
static struct hd_struct {
	long start_sect;
	long nr_sects;
} hd[5*1]={{0,0},};




void sys_setup(void *hard_disk)
{
    //读取硬盘参数
    hd_info.cyl = *(unsigned short *) hard_disk;
    hd_info.head = *(unsigned char *) (2+hard_disk);
    hd_info.wpcom = *(unsigned short *) (5+hard_disk);
    hd_info.ctl = *(unsigned char *) (8+hard_disk);
    hd_info.lzone = *(unsigned short *) (12+hard_disk);
    hd_info.sect = *(unsigned char *) (14+hard_disk);

    for (int i=0 ; i<1 ; i++) {
		hd[i*5].start_sect = 0;
		hd[i*5].nr_sects = hd_info.head*
				hd_info.sect*hd_info.cyl;
	}
    //读取分区表


    //加载虚拟盘？？？
    //加载根文件系统
}

void show_hard_info()
{
    printf("cylinder: %d\n", hd_info.cyl);
    printf("head: %d\n", hd_info.head);
    printf("wpcom: %d\n", hd_info.wpcom);
    printf("ctl: %d\n",hd_info.ctl);
    printf("lzone: %d\n",hd_info.lzone);
    printf("sect: %d\n",hd_info.sect);
}

//判断并且循环等待硬盘控制器置位
static int controller_ready(void)
{
	int retries=10000;

	while (--retries && (inb_p(0x1f7)&0xc0)!=0x40);
	return (retries);
}   

//检测硬盘执行命令后的状态。
static int win_result(void)
{
	int i=inb_p(HD_STATUS);

	if ((i & (BUSY_STAT | READY_STAT | WRERR_STAT | SEEK_STAT | ERR_STAT))
		== (READY_STAT | SEEK_STAT))
		return(0); /* ok */
	if (i&1) i=inb(HD_ERROR);
	printf("error:%x",i);
	return (1);
}

void show_hd_error()
{
	int i = inb_p(HD_ERROR);
	printf("error:%x\n",i);
}

void show_hd_status(void)
{
	int i=inb_p(HD_STATUS);
	printf("status:%x\n",i);
}

void hd_intr()
{
    printf("hdhd\n");
}

//向硬盘控制器发送命令
static void hd_out(unsigned int drive,unsigned int nsect,unsigned int sect,
		unsigned int head,unsigned int cyl,unsigned int cmd,
		void (*intr_addr)(void))
{
	printf("command:%x\n",cmd);
	register int port asm("dx");

	if (drive>1 || head>15)
		printf("Trying to write bad sector");
	if (!controller_ready())
		printf("HD controller not ready");
	do_hd = intr_addr;
	outb_p(hd_info.ctl,HD_CMD);
	port=HD_DATA;
	outb_p(hd_info.wpcom>>2,++port);
	outb_p(nsect,++port);
	outb_p(sect,++port);
	outb_p(cyl,++port);
	outb_p(cyl>>8,++port);
	outb_p(0xA0|(drive<<4)|head,++port);
	outb(cmd,++port);
}

//等待硬盘就绪
static int drive_busy(void)
{
	unsigned int i;

	for (i = 0; i < 10000; i++)
		if (READY_STAT == (inb_p(HD_STATUS) & (BUSY_STAT|READY_STAT)))
			break;
	i = inb(HD_STATUS);
	i &= BUSY_STAT | READY_STAT | SEEK_STAT;
	if (i == READY_STAT | SEEK_STAT)
		return(0);
	printf("HD controller times out\n\r");
	return(1);
}

//复位硬盘控制器
static void reset_controller(void)
{
	int	i;

	outb_p(4,HD_CMD);
	for(i = 0; i < 100; i++) nop();
	outb_p(hd_info.ctl & 0x0f ,HD_CMD);
	if (drive_busy())
		printf("HD-controller still busy\n");
	if ((i = inb(HD_ERROR)) != 1)
		printf("HD-controller reset failed: %x\n",i);
}

//复位硬盘
static void reset_hd(int nr)
{
	reset_controller();
	hd_out(nr,hd_info.sect,hd_info.sect,hd_info.head-1,hd_info.cyl,WIN_SPECIFY,&recal_intr);
}

//意外中断调用函数
void unexpected_hd_interrupt(void)
{
	printf("Unexpected HD interrupt\n\r");
}

//读取硬盘失败调用函数
static void bad_rw_intr(void)
{
	printf("read error");
	// if (++CURRENT->errors >= MAX_ERRORS)
	// 	end_request(0);
	// if (CURRENT->errors > MAX_ERRORS/2)
	// 	reset = 1;
}

//读取扇区中断调用函数
static void read_intr(void)
{
	if (win_result()) {
		bad_rw_intr();
		do_hd_request();
		return;
	}
	port_read(HD_DATA,CURRENT->buffer,256);

	//这里不是很明白为什么输出这么多位
	//printf("%x",(char)CURRENT->buffer[501]);
	
    //printf("sdfasdf");  
	CURRENT->errors = 0;
	CURRENT->buffer += 512;
	CURRENT->sector++;
	//printf("%d\n",CURRENT->nr_sectors);	
	if (--CURRENT->nr_sectors) {
		do_hd = &read_intr;
		return;
	}

	printf("read end\n");
	end_request(1);
	do_hd_request();
}

//写扇区中断调用函数
static void write_intr(void)
{
	if (win_result()) {
		bad_rw_intr();
		do_hd_request();
		return;
	}
	if (--CURRENT->nr_sectors) {
		CURRENT->sector++;
		CURRENT->buffer += 512;
		do_hd = &write_intr;
		port_write(HD_DATA,CURRENT->buffer,256);
		return;
	}
	printf("write end\n");
	end_request(1);
	do_hd_request();
}

//硬盘中断服务程序中调用的重新校正(复位)函数。
static void recal_intr(void)
{
	if (win_result())
		bad_rw_intr();
	else
	{
		printf("reset successful\n");
	}
	//do_hd_request();
}

void read_block(int block)
{    
	unsigned int sec,head,cyl;
	unsigned int nsect = 1;

    //通过绝对扇区数计算出柱面、磁道、扇区等参数
	__asm__("divl %4":"=a" (block),"=d" (sec):"0" (block),"1" (0),
		"r" (hd_info.sect));
	__asm__("divl %4":"=a" (cyl),"=d" (head):"0" (block),"1" (0),
		"r" (hd_info.head));
	sec++;
	hd_out(0,nsect,sec,head,cyl,WIN_READ,&read_intr);
}

//执行硬盘读写请求操作
void do_hd_request()
{
    
    int i,r;
	unsigned int block,dev;
	unsigned int sec,head,cyl;
	unsigned int nsect = 1;

    if (CURRENT->nr_sectors == 0)
	{
		printf("no request");
		return ;
	}

    //得到子设备号：分区
	dev = (CURRENT->dev & 0xff);
    //起始扇区
	block = CURRENT->sector;
    //防止出错
	// if (dev >= 5*NR_HD || block+2 > hd[dev].nr_sects) {
	// 	end_request(0);
	// 	goto repeat;
	// }

	block += hd[dev].start_sect;
	dev /= 5;
    //通过绝对扇区数计算出柱面、磁道、扇区等参数
	__asm__("divl %4":"=a" (block),"=d" (sec):"0" (block),"1" (0),
		"r" (hd_info.sect));
	__asm__("divl %4":"=a" (cyl),"=d" (head):"0" (block),"1" (0),
		"r" (hd_info.head));
	sec++;
	nsect = CURRENT->nr_sectors;
	if (CURRENT->cmd == WRITE) {
		hd_out(dev,nsect,sec,head,cyl,WIN_WRITE,&write_intr);
		for(i=0 ; i<3000 && !(r=inb_p(HD_STATUS)&DRQ_STAT) ; i++)
			// nothing  ;
		if (!r) {
			bad_rw_intr();
			return ;
            //goto repeat;
		}
		port_write(HD_DATA,CURRENT->buffer,256);
	} 
	else if (CURRENT->cmd == READ) {
		hd_out(dev,nsect,sec,head,cyl,WIN_READ,&read_intr);
	} 
	else
		printf("unknown hd-command");
        /**/
}

//结束请求
void end_request(int uptodate)
{
	//DEVICE_OFF(CURRENT->dev);
	if (CURRENT->bh) {
		CURRENT->bh->b_uptodate = uptodate;
		//unlock_buffer(CURRENT->bh);
	}
	// if (!uptodate) {
	// 	printf(DEVICE_NAME " I/O error\n\r");
	// 	printf("dev %04x, block %d\n\r",CURRENT->dev,
	// 		CURRENT->bh->b_blocknr);
	// }
	// wake_up(&CURRENT->waiting);
	// wake_up(&wait_for_request);
	// CURRENT = CURRENT->next;
}

//硬盘系统初始化
void hd_init(void)
{
	//blk_dev[MAJOR_NR].request_fn = DEVICE_REQUEST;
	//set_intr_gate(0x2E,&hd_interrupt);   // 设置中断门中处理函数指针
    outb_p(inb_p(PIC0_IMR)&0xfb,PIC0_IMR);       // 复位主片上接联引脚屏蔽位(位 2)。
    outb_p(inb_p(PIC1_IMR)&0xbf,PIC1_IMR);       // 复位从片上硬盘中断请求屏蔽位(位 6)。
}

void add_request(int dev, int wr, int blocks, void * buf)
{
	CURRENT->cmd = wr;//先只用读磁盘调用
	CURRENT->dev = dev;
	CURRENT->nr_sectors = 2;
	CURRENT->sector = blocks << 1; //块号转换为扇区号
	CURRENT->buffer = buf;
	do_hd_request();
	for(int i=1;i<10000;i++);
}


void read_disk(int blocks, void * buf)
{
    add_request(1, READ, blocks, buf);
}

void write_disk(int b, void * buf)
{
    add_request(1, WRITE, b, buf);
}

void test_hard_disk()
{
    //读取硬盘信息
    void *hard_disk = (void *)0x5f700;
    sys_setup(hard_disk);
    show_hard_info();

    //初始化硬盘控制器——开启中断
    hd_init();

	//检测是否是at硬盘控制器
	int cmos_disks;
	int NR_HD;
	if ((cmos_disks = CMOS_READ(0x12)) & 0xf0)
		if (cmos_disks & 0x0f)
			NR_HD = 2;
		else
			NR_HD = 1;
	else
		NR_HD = 0;
	printf("NR_HD:%d\n",NR_HD);

	//复位磁盘控制器
	reset_hd(0);
	
	//读取分区表
	CURRENT = mem_alloc(sizeof(struct request));

    //读写测试
    // void * buf = mem_alloc(1024);
    // *((char *)buf) = 98;
    // write_disk(1, buf);

    // void * buf2 = mem_alloc(1024);
    // printf("old buf2:%d\n", *((char *)buf2));
    // read_disk(1, buf2);
    // printf("new buf2:%d\n", *((char *)buf2));

    //

	// char * buf = mem_alloc(1024);
	// add_request(0, 0, 0, buf);
	// printf("%x %x",buf[510], buf[511]);
	
    // struct partition * p = 0x1BE + (void *)buf;
	// for(int i=1; i<5; i++, p++){
	// 	hd[i].start_sect = p->start_sect;
	// 	hd[i].nr_sects = p->nr_sects;
	// 	printf("start:%d n:%d\n",hd[i].start_sect, hd[i].nr_sects);
	// }

	//读磁盘检测
	
	// void * buf = mem_alloc(1024);
	// add_request(0, 0, buf);
	// //printf("%x%x",CURRENT->buffer[510],CURRENT->buffer[511]);

	// for(int i = 0;i < 10000;i++);
	// printf("%x",*((char *)buf+510));
    printf("\n\nend\n\n");
    //for(;;){;}
}