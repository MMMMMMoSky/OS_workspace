.code16                             		# 16-real mode
.text

.global _start                  		# export symbol for linking

.equ BOOTSEG, 0x07c0               			# segment address
.equ SETUPSEG, 0x8000


        ljmp $BOOTSEG, $_start
_start:
        # Get cursor position (dl, dh)
        mov $0x03, %ah
        int $0x10
        # Print string: Loading setup program...
        mov $BOOTSEG, %ax
        mov %ax, %es
        mov $str_loading_setup, %bp
        mov $0x1301, %ax
        mov $0x000b, %bx
        mov $24, %cx						# length of string
        int $0x10
load_setup:
# 1. load setup.S to [0x80000, 0x80800)
        # load 2nd ~ 5th sector to [0x80000, 0x80800)
        mov $0x0000, %dx					# Drive 0 (A, floppy), Head 0
        mov $0x0002, %cx					# C0S2 (sector indexed from 1)
        mov $SETUPSEG, %ax					# load to ES:BX
        mov %ax, %es
        mov $0x00, %bx		
        mov $4, %al						# read 4 sectors
        mov $02, %ah						
        int $0x13				

        jnc setup_load_ok					# success

        mov $0x0000, %dx
        mov $0x0000, %ax					# Reset the Disk
        int $0x13
        jmp load_setup						# retry

setup_load_ok:
# 2. jump to setup.S
        # Print 'ok\r\n'
        mov $0x03, %ah
        int $0x10
        mov $BOOTSEG, %ax
        mov %ax, %es
        mov $str_ok, %bp
        mov $0x1301, %ax
        mov $0x000b, %bx
        mov $4, %cx						# length of string
        int $0x10

        # jump to setup.S (0x80000)
        mov $SETUPSEG, %ax 				# move data segment
        mov %ax, %ds				
        ljmp $SETUPSEG, $0				# move code segment and ip (do not use mov)


str_loading_setup:
        .ascii "Loading setup program..."
str_ok:
        .ascii "ok\r\n"

.= 510                              # fill zeros
.word 0xaa55                		# 55aa (small end)
