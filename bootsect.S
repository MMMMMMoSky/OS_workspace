.code16                             # 16-real mode
.global _bootstart                  # export symbol for linking
.equ BOOTSEG, 0x07c0                # segment address

.text

ljmp $BOOTSEG, $_bootstart

_bootstart:
		# get cursor position (dl, dh)
		mov $0x03, %ah
		int $0x10

		mov $BOOTSEG, %ax
		mov %ax, %es
		mov $_string, %bp
		mov $0x1301, %ax
		mov $0x000b, %bx
		mov $19, %cx
		int $0x10

hltloop:
		hlt
		jmp hltloop

_string:
		.ascii "Hello Bootloader!\r\n"




.= 510                              # fill zeros
.word 0xaa55                		# 55aa (small end)
