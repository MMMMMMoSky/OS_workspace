.PHONY=clean run all
CC=gcc 
CFLAGS=-m32 -fomit-frame-pointer -fno-pie -fno-stack-protector

all: Image

run: Image
	@qemu-system-i386 -boot a -fda Image

bootsect.o: bootsect.S
	@as --32 bootsect.S -o bootsect.o

bootsect: bootsect.o ld_script.ld
	@ld -T ld_script.ld bootsect.o -o bootsect
	@objcopy -O binary -j .text bootsect

setup.o: setup.S 
	@as --32 setup.S -o setup.o

setup: setup.o ld_script.ld
	@ld -T ld_script.ld setup.o -o setup
	@objcopy -O binary -j .text setup

sys_head.o: sys_head.S 
	@as --32 sys_head.S -o sys_head.o

os_main.o: os_main.c
	@$(CC) -c os_main.c -o os_main.o $(CFLAGS)

os_main.s: os_main.c
	@$(CC) -S os_main.c -o os_main.s $(CFLAGS)

system: sys_head.o os_main.o
	@ld -T ld_script.ld sys_head.o os_main.o -o system
	#@objcopy -O binary -j .text system 改为下面的,就可以用全局变量了
	@objcopy -O binary -R .note -R .comment system 


Image: bootsect setup system
	@dd if=bootsect of=Image bs=512 count=1
	@dd if=setup of=Image bs=512 count=4 seek=1
	@dd if=system of=Image bs=512 count=4 seek=5  
	@echo "Image built done"

clean:
	@rm -f *.o bootsect setup Image sys_head os_main os_main.s system
