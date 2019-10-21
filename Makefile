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

sys_head: sys_head.o ld_script.ld
	@ld -T ld_script.ld sys_head.o -o sys_head
	@objcopy -O binary -j .text sys_head

Image: bootsect setup sys_head
	@dd if=bootsect of=Image bs=512 count=1
	@dd if=setup of=Image bs=512 count=4 seek=1
	# 暂时添加 sys_head, 至少不没地方跳
	@dd if=sys_head of=Image bs=512 seek=5  
	@echo "Image built done"

clean:
	@rm -f *.o bootsect setup Image sys_head
