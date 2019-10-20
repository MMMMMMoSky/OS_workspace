.code16
.text 

.equ SETUPSEG, 0x0800                       # must be the same as bootsect.S

        # Print string: Executing setup program
        mov $0x03, %ah
        int $0x10
        mov $SETUPSEG, %ax
        mov %ax, %es
        mov $str_exec_setup, %bp
        mov $0x1301, %ax
        mov $0x000b, %bx
        mov $26, %cx                        # length of string
        int $0x10

hltloop:
        hlt
        jmp hltloop

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
