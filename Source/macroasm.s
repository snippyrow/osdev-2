; Print a string to console in real 16-bit emulation mode

%macro printf_rmode 1
    mov si, %1 ; Move pointer for string into SI
    call p_start ; Call the printing subroutine (escape from macro)
    ; Remember, assembly includes paste the code *directly*, so p_start is contained at the end of the bootloader code.
%endmacro

