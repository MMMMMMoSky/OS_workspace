.code16
.text 

.equ SETUPSEG, 0x8000                       # must be the same as bootsect.S
.equ SYSINITSEG, 0x1000                     # the begining position of system

_start:
        # Set stack pointer 0x8900:0xff
        mov $0x8900, %ax
        mov %ax, %ss
        mov $0xff, %ax 
        mov %ax, %sp 
        # Set Data Segment
        mov $SETUPSEG, %ax
        mov %ax, %ds

        # Print string: execute setup
        mov $str_exec_setup, %ax
        mov $26, %cx                        # length of string
        call print_str

# 1. Load System to 0x10000
        # INT 0x13 AH = 0x02
        # AL: Number of sectors; DH: head, DL: Drive
        # ES:BX: Buffer address pointer
        # CH: Cylinder; CL: Sector
        # CX =       ---CH--- ---CL---
        # cylinder:  76543210 98
        # sector:               543210
        # AL(return): Actually number of sectors read

        # Print string: load system
        mov $str_load_system, %ax
        mov $17, %cx
        call print_str
        # Load C0-H0-S6 ~ C0-H0-S18 to [0x10000, 0x11a00)
        mov $0x0000, %dx                        # Drive 0 (A, floppy), Head 0
        mov $0x0006, %cx                        # C0S6 (sector indexed from 1)
        mov $SYSINITSEG, %ax                    # load to ES:BX
        mov %ax, %es
        mov $0x00, %bx
        mov $13, %al                            # read 13 sectors
        mov $02, %ah						
        int $0x13				
        jc load_system_failed
        cmp $13, %al 
        jne load_system_failed
        # Load C0-H1-S1 ~ C0-H1-S18 to [0x11a00, 0x13e00)
        mov $0x0100, %dx                        # Drive 0 (A, floppy), Head 1
        mov $0x0006, %cx                        # C0S6 (sector indexed from 1)
        mov $SYSINITSEG, %ax                    # load to ES:BX
        add $0x1a0, %ax                         # have read 13 sectors (segment address += 0xd * 0x20 >> 0x04)
        mov %ax, %es                            # calc addr for we can change SYSINITSEG without modifing codes here
        mov $0x00, %bx
        mov $18, %al                            # read 18 sectors
        mov $02, %ah						
        int $0x13				
        jc load_system_failed
        cmp $18, %al 
        jne load_system_failed
        # Load C1 ~ C20, totally 20 cylinders (360kb)
        mov $SYSINITSEG, %ax
        add $0x3e0, %ax                         # we have read 31 sectors
        push %ax                                # start address
        push $0x0100                            # cylinder and sector
        push $0x0000                            # head and drive
load_c1_c20:
        # DEBUG
        # Print string: load ok
        mov $str_ok, %ax
        mov $4, %cx
        call print_str

        pop %dx
        pop %cx
        pop %es
        cmp $21, %ch
        je load_system_ok
        mov $0x00, %dl
        mov $0x01, %cl
        mov $18, %al
        mov $02, %ah						
        int $0x13				
        jc load_system_failed
        cmp $18, %al 
        jne load_system_failed
        mov %es, %ax                            # address add 18 * 512 >> 4 = 0x240
        add $0x240, %ax
        push %ax
        mov %dx, %ax
        and $0x0100, %ax
        add %ax, %cx                            # cylinder add 1 when head is 1
        push %cx
        xor $0x0100, %dx                        # head should reverse
        push %dx
        jmp load_c1_c20

load_system_ok:
        # Print string: load ok
        mov $str_ok, %ax
        mov $4, %cx
        call print_str


hltloop:
        hlt
        jmp hltloop

load_system_failed:
        # Print string: failed
        mov $str_failed_load_sys, %ax
        mov $51, %cx
        call print_str
        
        jmp hltloop

# print a string, ds:ax, length: cx
.type print_str, @function
print_str:
        push %bp
        push %es
        push %bx
        push %dx

        push %ax
        push %cx 
        mov $0x03, %ah
        int $0x10
        pop %cx
        pop %bp
        mov %ds, %ax
        mov %ax, %es
        mov $0x1301, %ax
        mov $0x000b, %bx
        int $0x10

        pop %dx 
        pop %bx
        pop %es
        pop %bp
        ret

str_exec_setup:
        .ascii "Executing setup program:\n\r"

str_load_system:
        .ascii "Loading system..."

str_failed_load_sys:
        .ascii "Failed\n\r\n\rCheck your disk/floppy or try to restart!"

str_ok:
        .ascii "ok\n\r"


# May be useful later

# # Extend momory store
# # 0x15 0x88
# 		mov $0x88, %ah
# 		int $0x15
# 		mov %ax, %ds:2

# # Enable A20
# 		in $0x92, %al
# 		or $0x02, %al 
# 		out %al, $0x92

# # VGA Display Mode 
# 		mov $0x12, %ah 
# 		mov $0x10, %bl 
# 		int $0x10 
# 		mov %ax, %ds:8
# 		mov %bx, %ds:10
# 		mov %cx, %ds:12

# # Harddisk Parameter Table
# 		mov $0x0000, %ax
# 		mov %ax, %ds 
# 		lds %ds:4*0x41, %si 
# 		mov %ax, %es 
# 		mov $0x0080, %di 
# 		mov $0x10, %cx 
# 		rep movsb 

# 		mov $0x0000, %ax 
# 		mov %ax, %ds
# 		lds %ds:4*0x46, %si 
# 		mov $INITSEG, %ax 
# 		mov %ax, %es 
# 		mov $0x0090, %di 
# 		mov $0x10, %cx 
# 		rep movsb
