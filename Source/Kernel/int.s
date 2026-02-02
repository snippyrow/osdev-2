[bits 32]
[global isr80_stub]
[global kbd_int_stub]
[extern syscall_handle]
[extern kbd_int_handle]

; Interrupt code, since it's often easier to use some minimal seembly wrappers for them.

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

kbd_int_stub:
    pusha

    call kbd_int_handle

    mov al, 0x20
    out 0x20, al
    popa
    iret