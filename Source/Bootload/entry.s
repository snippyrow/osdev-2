[bits 32]
[extern kmain]
[global _start]

section .text

_start:
    cli ; Clear interrupts before starting
    
    call kmain ; Call kernel
    hlt
    jmp $ ; Halt in case of exception/return