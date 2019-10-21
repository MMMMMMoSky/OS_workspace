.code16
.text 

.equ SETUPSEG, 0x8000                       # must be the same as bootsect.S
.equ SYSINITSEG, 0x1000                     # the begining position of system
.equ SYSSIZE, 0x59600                       # C0-H0-S6 ~ C19-H1-S18, 357kb

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

        # Load C0-H0-S6 ~ C19-H1-S18 to [0x10000, 0x69600)
        mov $0x0000, %dx                        # Drive 0 (A, floppy), Head 0
        mov $0x0006, %cx                        # C0S6 (sector indexed from 1)
        mov $SYSINITSEG, %ax                    # (SYSINITSEG can be changed without modifying codes below)
        mov %ax, %es                            # read to ES:BX
        mov $0x0000, %bx                        # BX will always be 0
load_system_loop:
        mov $0x0201, %ax
        int $0x13                               # read a sector
        jc load_system_failed
        cmp $1, %al 
        jne load_system_failed
        mov %es, %ax                            # address add 512B
        add $0x20, %ax
        mov %ax, %es
        add $1, %cl                             # sector add 1
        cmp $19, %cl
        jne load_system_loop
        mov $1, %cl
        add $1, %dh                             # head add 1
        cmp $2, %dh
        jne load_system_loop
        mov $0, %dh 
        add $1, %ch                             # cylinder add 1
        cmp $20, %ch
        jne load_system_loop

        # Print string: load ok
        mov $str_ok, %ax
        mov $2, %cx
        call print_str
        call print_endl

# 2. Enable A20 mode
        # Print string: enable a20
        mov $str_enableA20, %ax
        mov $18, %cx
        call print_str

        in $0x92, %al
        or $0x02, %al 
        out %al, $0x92

        # Check memory, returns bx in units of 64kb
        mov $0xe801, %ax 
        int $0x15
        jc enable_a20_failed

        shr $10, %ax                    # calc memory size
        shr $4, %bx
        add %ax, %bx

        # Print string: ok
        mov $str_ok, %ax
        mov $2, %cx
        call print_str
        call print_space
        
        call print_hex                  # print memory size
        call print_space
        #Print string: Mb memory total
        mov $str_mb_memory, %ax
        mov $18, %cx
        call print_str

# 3. VGA 0x12 640x280x16
        mov $str_enter_vga, %ax
        mov $30, %cx
        call print_str

        # VGA Mode
        mov $0x0012, %ax
        int $0x10

        # FIXME 可能还有一些参数没有保存

        cli                     # clear interrupt bit (close int)
# 4. Move System to 0x00000 ~ SYSSIZE
        # Move [0x10000, 0x80000) to [0x00000, 0x70000)
        # movsw from ds:si to es:di
        cld                     # d = 0, si/di++
        mov $0x0000, %ax
move_repeat:
        mov $0, %di
        mov $0, %si
        mov $0x8000, %cx        # 64Kb each loop
        mov %ax, %es            
        add $0x1000, %ax        # 巧妙, 刚好+0x1000, 还能判断
        mov %ax, %ds            
        mov $0x8000, %cx        # movsw 0x8000 word = 0x10000 bytes
        rep movsw
        cmp $0x8000, %ax        
        jne move_repeat
        # move finished
# 5. Set up GDT, IDT, PIC
        # GDT, IDT
        mov $SETUPSEG, %ax 
        mov %ax, %ds 
        lidt idtr_info
        lgdt gdtr_info
        # PIC
        mov $0x11, %al			# Init ICW1, 0x11 is init command
        out %al, $0x20			# 0x20 is 8259A-1 Port
        .word 0x00eb, 0x00eb	        # Time Delay jmp $+2, jmp $+2
        out %al, $0xA0			# And init 8259A-2
        .word 0x00eb, 0x00eb
        mov $0x20, %al			# Send Hardware start intterupt number(0x20)
        out %al, $0x21			# From 0x20 - 0x27
        .word 0x00eb, 0x00eb
        mov $0x28, %al
        out %al, $0xA1			# From 0x28 - 0x2F
        .word 0x00eb, 0x00eb
        mov $0x04, %al			# 8259A-1 Set to Master
        out %al, $0x21
        .word 0x00eb, 0x00eb
        mov $0x02, %al			# 8259A-2 Set to Slave
        out %al, $0xA1
        .word 0x00eb, 0x00eb
        mov $0x01, %al			# 8086 Mode
        out %al, $0x21
        .word 0x00eb, 0x00eb
        out %al, $0xA1
        .word 0x00eb, 0x00eb
        mov $0xFF, %al
        out %al, $0x21			# Mask all the interrupts now
        .word 0x00eb, 0x00eb
        out %al, $0xA1			
# 6 Enable Protected Mode
	mov %cr0, %eax
	bts $0, %eax		        # Turn on Protect Enable (PE) bit
	mov %eax, %cr0

        ljmp $0, $0                     # Jump to start point

hltloop:
        hlt
        jmp hltloop

enable_a20_failed:
        # Print string: enable a20 failed
        mov $str_enableA20_failed, %ax
        mov $13, %cx
        call print_str
        jmp hltloop

load_system_failed:
        # Print string: failed
        mov $str_failed_load_sys, %ax
        mov $51, %cx
        call print_str
        jmp hltloop

# print values in bx
.type print_hex, @function
print_hex:
        push %ax
        push %bx
        push %cx
        push %dx
        pushf
        mov %bx, %dx
        mov $0x0007, %bx

        mov $0x0e, %ah
        mov $'0', %al
        int $0x10
        mov $'x', %al
        int $0x10

        mov $12, %cl
print_each_digit:
        mov %dx, %ax
        shr %cl, %ax
        and $0x0f, %al
        cmp $9, %al
        jg hex_letter
        add $'0', %al
        jmp print_a_digit
hex_letter:
        add $'a', %ax
        sub $10, %ax
print_a_digit:
        mov $0x0e, %ah
        int $0x10
        cmp $0, %cl
        je print_hex_end
        sub $4, %cl
        jmp  print_each_digit

print_hex_end:
        popf
        pop %dx
        pop %cx
        pop %bx
        pop %ax
        ret

.type print_endl, @function
print_endl:
        push %ax
        push %bx
        mov $0x000b, %bx
        mov $0x0e, %ah
        mov $0x0a, %al
        int $0x10
        mov $0x0d, %al
        int $0x10
        pop %bx
        pop %ax
        ret

.type print_space, @function
print_space:
        push %ax
        push %bx
        mov $0x000b, %bx
        mov $0x0e, %ah
        mov $0x20, %al
        int $0x10
        pop %bx
        pop %ax
        ret


# print a string, ds:ax, length: cx
.type print_str, @function
print_str:
        push %bp
        push %es
        push %ax
        push %bx
        push %cx
        push %dx

        push %ax
        push %cx 
        mov $0x0000, %bx                # TODO: Maybe a bug
        mov $0x03, %ah
        int $0x10
        pop %cx
        pop %bp
        mov %ds, %ax
        mov %ax, %es
        mov $0x1301, %ax
        mov $0x0007, %bx
        int $0x10

        pop %dx 
        pop %cx
        pop %bx
        pop %ax
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
        .ascii "ok"

str_enableA20:
        .ascii "Enable A20 mode..."
        
str_enableA20_failed:
        .ascii "Failed\n\r\n\rMemory error."

str_mb_memory:
        .ascii "Mb memory total.\n\r"
str_enter_vga:
        .ascii "Entering VGA display mode...\n\r"

gdtr_info:
        .word 0x800
        .word 512+gdt_info, 0x9
idtr_info:
        .word 0
        .word 0, 0
gdt_info:
        .word 0,0,0,0

        .word 0x07ff
        .word 0x0000
        .word 0x9a00
        .word 0x00c0

        .word 0x07ff
        .word 0x0000
        .word 0x9200
        .word 0x00c0

# May be useful later

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
