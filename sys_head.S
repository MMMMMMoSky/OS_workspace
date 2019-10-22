.code16

        mov $0xa000, %bx 
        mov %bx, %ds
        mov $0, %ecx 
putloop:
        movb $0xff, (%ecx)
        add $0x01, %ecx
        cmp $0xffff, %ecx 
        jne putloop

hltloop:
        hlt 
        jmp hltloop