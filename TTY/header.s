[bits 32]

; Enviornment/loader variables for the kernel loader
headers:
    entry_sym: dd _start

_start:
    ; call main ; Call primary C/C++ program.
    ret ; Return back to kernel.