[bits 16] ; Define type of assembly file
[org 0x7c00] ; Starting address of MBR

KERNEL equ 0x7e00 ; Address where the kernel actually begins

begin:
    mov ah, 0xE ; Load command byte
    mov al, 'Z' ; Character to print
    int 0x10 ; Print character
    jmp $ ; Halt program

; Magic boot signature at the end
times 510-($-$$) db 0
dw 0xaa55