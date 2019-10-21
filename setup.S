.code16
.text 

.equ SETUPSEG, 0x8000                       # must be the same as bootsect.S

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
        mov $28, %cx                        # length of string
        call print_str

hltloop:
        hlt
        jmp hltloop

# print a string, ds:ax, length: cx
.type print_str, @function
print_str:
        push %bp
        push %es

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

        push %es
        push %bp
        ret

str_exec_setup:
        .ascii "Executing setup program...\n\r"


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
