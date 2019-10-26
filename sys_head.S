.code32

.global main          # why is not _main ??
.global io_in8, io_in16, io_in32
.global io_out8, io_out16, io_out32
.global io_load_eflags, io_store_eflags
.global io_store_idtr, inthandler21
.extern print

	call main       # jump to os_main.c main()
loop:
	jmp loop


# some functions called in C language

.type io_in8, @function
io_in8:                 # int io_in8(int port)
    mov 4(%esp), %edx
    mov 0, %eax
    in %dx, %al
    ret 

.type io_in16, @function
io_in16:                # int io_in16(int port)
    mov 4(%esp), %edx
    mov 0, %eax
    in %dx, %ax
    ret 

.type io_in32, @function
io_in32:                # int io_in32(int port)
    mov 4(%esp), %edx
    in %dx, %eax
    ret 

.type io_out8, @function
io_out8:                # int io_out8(int port,  data)
    mov 4(%esp), %edx   # port
    mov 8(%esp), %eax   # data
    out %al, %dx
    ret 

.type io_out16, @function
io_out16:               # int io_out16(int port, int data)
    mov 4(%esp), %edx
    mov 8(%esp), %eax
    out %ax,%dx
    ret 

.type io_out32, @function
io_out32:               # int io_out32(int port, int data)
    mov 4(%esp), %edx
    mov 8(%esp), %eax
    out %eax, %dx
    ret 

.type io_load_eflags, @function
io_load_eflags:          # int io_load_eflags(void)
    pushf
    pop %eax
    ret 

.type io_store_eflags, @function
io_store_eflags:         # void io_store_eflags(int eflags)
    mov 4(%esp), %eax
    push %eax
    popf
    ret

.type io_store_idtr, @function
io_store_idtr:          # void io_store_idtr(int limit, int addr);
    mov 4(%esp), %ax
    mov %ax, 6(%esp)	
    lidt 6(%esp)
    ret

inthandler21:
	pushl %eax
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	
	mov $0x10, %eax
	mov %eax, %ds
	mov %eax, %es
	mov %eax, %fs
	call print
	
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret 
