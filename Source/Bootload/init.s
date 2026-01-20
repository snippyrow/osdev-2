[bits 16] ; Define type of assembly file
[org 0x7c00] ; Starting address of MBR

; Bootlaoder must enter protected mode and load kernel in 512 bytes..

KERNEL equ 0x7e00 ; Address where the kernel actually begins

%include "Source/macroasm.s"

begin:
    printf_rmode test_str
    jmp $ ; Halt program

[bits 16]
p_start:
    pusha
    mov ah, 0xe ; Command code to print a character
p_loop:
    lodsb
    cmp al, 0 ; Check if null character
    je p_done
    int 0x10 ; Send BIOS code
    jmp p_loop
p_done:
    popa
    ret

test_str:
    db "Welcome to the system!",10,13,0

; Magic boot signature at the end
times 510-($-$$) db 0
dw 0xaa55