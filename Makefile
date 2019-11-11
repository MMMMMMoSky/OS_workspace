.PHONY = clean run all
CC = gcc 
CFLAGS = -m32 -fomit-frame-pointer -fno-pie -fno-stack-protector -nostdlib -fno-builtin
C_OBJS = os_main.o mem_manage.o hardware_init.o text_video.o \
	console_io.o byte_buffer.o time.o terminal.o hard_disk.o \
	builtin_commands.o file.o
LDFLAGS += -m $(shell $(LD) -V | grep elf_i386 2>/dev/null | head -n 1)

all: Image

run-qemu: Image hard_disk_drive
	@qemu-system-i386 -boot a -fda Image -hda hard_disk_drive

run-bochs: Image hard_disk_drive
	@/usr/bin/bochs

# bootsect
bootsect.o: bootsect.asm
	@as --32 bootsect.asm -o bootsect.o
bootsect: bootsect.o ld_script.ld
	@ld -T ld_script.ld bootsect.o -o bootsect
	@objcopy -O binary -j .text bootsect

# setup program
setup.o: setup.asm 
	@as --32 setup.asm -o setup.o
setup: setup.o ld_script.ld
	@ld -T ld_script.ld setup.o -o setup
	@objcopy -O binary -j .text setup

# system head
sys_head.o: sys_head.asm 
	@as --32 sys_head.asm -o sys_head.o

# 针对 C 源代码 的编译
%.o: %.c
	@$(CC) -c $*.c -o $*.o $(CFLAGS)
%.s: %.c 
	@$(CC) -S $*.c -o $*.s $(CFLAGS)

# system kernel: all C sources
kernel: $(C_OBJS)
	@ld $(LDFLAGS) -r -N -o kernel $(C_OBJS)

# system: system head + kernel
system: sys_head.o kernel
	@ld -T ld_script.ld sys_head.o kernel -o system
	@objcopy -O binary -R .note -R .comment system 

# system Image: bootsect(1 sector) + setup(4 sectors) + system
Image: bootsect setup system
	@dd if=bootsect of=Image bs=512 count=1
	@dd if=setup of=Image bs=512 count=4 seek=1
	@dd if=system of=Image bs=512 count=100 seek=5  
	@echo "Image built done"

# make hard disk
hard_disk_drive:
	@qemu-img create hard_disk_drive 100M

clean:
	@rm -f *.o *.s bootsect setup sys_head kernel system Image hard_disk_drive
