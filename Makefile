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

Image: bootsect setup
	@dd if=bootsect of=Image bs=512 count=1
	@dd if=setup of=Image bs=512 count=4 seek=1
	@echo "Image built done"

clean:
	@rm -f *.o bootsect setup Image
