.code32

.global main            # why is not _main ??
.global asm_hlt

        call main

.type asm_hlt, @function
asm_hlt:               # void asm_hlt(void)
    hlt 
    ret
