.code16                             # 16-real mode
.global _bootstart                  # export symbol for linking

.equ BOOTSEG, 0x07c0                # segment address
.equ DEMOSEG, 0x1000

.text

ljmp $BOOTSEG, $_bootstart

_bootstart:
		# get cursor position (dl, dh)
		mov $0x03, %ah
		int $0x10

		# display a string
		mov $BOOTSEG, %ax
		mov %ax, %es
		mov $_string, %bp
		mov $0x1301, %ax
		mov $0x000b, %bx
		mov $19, %cx				# length of string
		int $0x10

_load_demo:							# load floppy disk
		mov $0x0000, %dx
		mov $0x0002, %cx
		mov $DEMOSEG, %ax
		mov %ax, %es 
		mov $0x0200, %bx 
		mov $0x02, %ah 
		mov $4, %al 
		int $0x13
		jc _load_demo				# retry if load failed

demo_load_ok:						# Jump to the demo program
		mov $DEMOSEG, %ax 
		mov %ax, %ds				# move data segment
		ljmp $0x1020, $0			# move code segment and ip (do not use mov)

_string:
		.ascii "Hello Bootloader!\r\n"




.= 510                              # fill zeros
.word 0xaa55                		# 55aa (small end)
