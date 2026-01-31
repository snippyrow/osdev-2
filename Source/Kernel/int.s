[bits 32]
[global isr80_stub]
[extern syscall_handle]

isr80_stub:
    ; CPU pushed: eflags, cs, eip  (12 bytes)

    ; Optional but common: save args registers so C can see them
    push edx
    push ecx
    push ebx
    push eax ; First arg pushed last

    ; Push pointer to the &frame
    mov eax, esp
    push eax

    ; Optional: add a ptr to the frame
    call syscall_handle
    add esp, 20 ; Remove everything

    iret