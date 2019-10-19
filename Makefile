all: Image

run: Image
	@qemu-system-i386 -boot a -fda Image

bootsect.o: bootsect.S
	@as --32 bootsect.S -o bootsect.o

bootsect: bootsect.o ld-bootsect.ld
	@ld -T ld-bootsect.ld bootsect.o -o bootsect
	@objcopy -O binary -j .text bootsect

demo.o: demo.S 
	@as --32 demo.S -o demo.o

demo: demo.o 
	@ld -T ld-bootsect.ld demo.o -o demo
	@objcopy -O binary -j .text demo

Image: bootsect demo
	@dd if=bootsect of=Image bs=512 count=1
	@dd if=demo of=Image bs=512 count=4 seek=1
	@echo "Image built done"

clean:
	@rm -f *.o bootsect demo Image
