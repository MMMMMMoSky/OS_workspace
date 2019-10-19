.code16

.text 

.equ DEMOSEG, 0x1020
.equ MSGLEN, 41

show_text:
	# get cursor position (dl, dh)
        mov $DEMOSEG, %ax
        mov %ax, %es
        mov $0x03, %ah
        xor %bh, %bh                    # mov 0 to bh
        int $0x10 

        mov $0x000a, %bx
        mov $0x1301, %ax 
        mov $MSGLEN, %cx
        mov $msg, %bp
        int $0x10

hltloop:
        hlt
        jmp hltloop

msg:
        .ascii "Successfully load the floppy into RAM\n\r\n\r"