.code32

.equ pg_dir, 0x70000
.equ pg0, 0x71000
.equ pg1, 0x72000
.equ pg2, 0x73000
.equ pg3, 0x74000

.global main          # why is not _main ??
.global io_in8, io_in16, io_in32
.global io_out8, io_out16, io_out32
.global io_load_eflags, io_store_eflags
.global io_store_idtr, inthandler21, inthandler20
.extern keyboard_intr, handle_IRQ0

    # call main       # jump to os_main.c main()
    jmp after_page_tables
loop:
	jmp loop

after_page_tables:
	pushl $0		# These are the parameters to main :-)
	pushl $0
	pushl $0
	pushl $L6		# return address for main, if it decides to.
	pushl $main
	jmp setup_paging
L6:
	jmp L6			# main should never return here, but
					# just in case, we know what happens.

setup_paging:
	movl $1024*5,%ecx		    /* 5 pages - pg_dir+4 page tables */
	xorl %eax,%eax
	mov  $pg_dir, %edi			/* pg_dir is at 0x70000 */
	cld;rep;stosl
	mov  $pg_dir, %edi
    mov  $pg0+7,%eax 
	movl %eax,(%edi)		    /* set present bit/user r/w */
    mov  $pg1+7,%eax 
	movl %eax,4(%edi)		    /*  --------- " " --------- */
    mov  $pg2+7,%eax 
	movl %eax,8(%edi)		    /*  --------- " " --------- */
    mov  $pg3+7,%eax 
	movl %eax,12(%edi) 			/*  --------- " " --------- */
	movl $pg3+4092,%edi
	movl $0xfff007,%eax		    /*  16Mb - 4096 + 7 (r/w user,p) */
	std
1:	stosl			/* fill pages backwards - more efficient :-) */
	subl $0x1000,%eax
	jge 1b
	mov  $pg_dir,%eax		    /* pg_dir is at 0x0000 */

b:	movl %eax,%cr3		        /* cr3 - page directory start */
	movl %cr0,%eax
	orl $0x80000000,%eax
	movl %eax,%cr0		        /* set paging (PG) bit */
	ret



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
	call keyboard_intr
	
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret 

inthandler20:
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
	call handle_IRQ0
	
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret 
