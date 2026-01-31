[bits 32]
[global k_call]

k_call:
    mov eax, [esp + 4] ; Sysno
    mov ebx, [esp + 8] ; a0
    mov ecx, [esp + 12] ; a1
    mov edx, [esp + 16] ; a2
    
    ; First argument pushed last
    push edx
    push ecx
    push ebx
    push eax

    int 0x80

    add esp, 16

    ; Return placed into EAX by syscall stub
    ret ; Return back to kernel.