[bits 16] ; Define type of assembly file
[org 0x7c00] ; Starting address of MBR

; Bootlaoder must enter protected mode and load kernel in 512 bytes..

; This section of code is the MBR, automatically loaded into 0x7c00. Starts at sector 1.

VBE_INFO equ 0x6000 ; Pointer to save basic VESA controller cababilities
MODE_INFO equ 0x6200 ; Info about the specific mode we're using
KERNEL equ 0x7e00 ; Address where the kernel actually begins

VIDEO_DESIRED equ 0x112 ; 640x480, 24bpp (Most GPU's support this implicitly.)
SP_TOP equ 0x100000 ; Sufficiently high. If the stack reaches the code segment, something has gone very wrong.

FAT_FS_SIZE equ 2 << 27 ; Size of FAT in bytes (256MB)


jmp begin
nop

; Code for MBR of the FAT32 filesystem. (Little-endian only!!)
; https://wiki.osdev.org/FAT
FAT32_BPB:
    db "MSWIN4.1" ; OEM Identifier. Mostly useless, but some drivers expect that value "MSWIN4.1"
    dw 512 ; Bytes per sector
    db 1 ; sectors per cluster (1)
    dw 32 ; Reserved sector count (this seems to be a good default..?) I guess it keeps room for boot sectors
    db 2 ; Number of file allocation tables.
    dw 0 ; Unused (number of root directory entires)
    dw 0 ; Number of total sectors. Usually 0 for FAT32
    db 0xf8 ; Media type. Set to fixed disk partition (hard disk)
    dw 0 ; Unused in FAT32

    ; CHS Geometry
    dw 32 ; Sectors per track
    dw 16 ; Number of heads
    dd 0 ; Hidden sectors (LBA offset)
    dd (FAT_FS_SIZE / 512) ; 32-bit total sector count

    dd (FAT_FS_SIZE / 512 / 128) - 63 ; Sectors per FAT. 512 means a 32MB volume. (Formula: SpF * 128 * 512 = VOLUME)
    dw 0 ; Flags
    dw 0 ; FAT version
    dd 2 ; Root directory (First cluster)
    dw 1 ; FSInfo sector (1)
    dw 6 ; Backup boot sector. I'll probably keep this unused..
    times 12 db 0 ; Reserved

    db 0x80 ; Drive number
    db 0 ; Reserved
    db 0x29 ; Extended boot sector
    dd 0x12345678 ; Volume serial no.
    db "VARA FAT32!" ; 11-byte filesystem label
    db "FAT32   " ; FS type.

; Packet to use for using int 0x13 extended read, as per OSDEVER.
DISK_ADDR_PCKT:
    db 0x10 ; Size of packet
    db 0 ; Reserved
    dw 50 ; Number of sectors to transfer
    dw 0 ; Offset of transfer buffer
    dw KERNEL >> 4 ; Segment of transfer buffer
    dd FAT_FS_SIZE / 512 ; Lower LBA
    dd 0 ; Higher LBA


%include "Source/Bootload/macroasm.s"

section .text

begin:
    printf_rmode test_str

    mov ah, 0 ; Issue a disk reset here
    int 0x13
    
    ; Perform an LBA extended read outside of the FAT32 FS.
    ; Check extended LBA support
    mov ah, 0x41
    mov dl, 0x80
    mov bx, 0x55aa
    int 0x13
    jc mbr_err ; Not supported, terminate

    ; Perform the read
    xor ax, ax
    mov ds, ax ; Clear DS for safety
    mov si, DISK_ADDR_PCKT ; Set DS:DI
    mov ah, 0x42
    mov dl, 0x80 ; Primary drive

    int 0x13 ; Call interrupt for reading HDD

    jc mbr_err

    ; Begin transferring to protected mode (32-bit operating)

    ; Enable the A20 memory wraparound, using FAST A20 option.
    ; When using older systems, probe keyboard controller instead.
    in al, 0x92
    or al, 2
    out 0x92, al

    ; Firstly, find all compatible video modes that the BIOS video controller supports.
    ; BIOS would have enumerated to find GPU's already, so pick something desirable.

    ; Fetch video capabilities, save to a block at 0x6000
    mov ax, VBE_INFO >> 4
    mov es, ax
    xor di, di ; Clear DI
    mov ax, 0x4f00 ; Load code for reading controller
    int 0x10

    ; If al /= 0x4F, something went wrong with reading the controller. Terminate.
    cmp al, 0x4f
    jne mbr_err

    ; Load mode list ptr. Overwrite the VBE_INFO block with our new list, discard the rest.
    mov bx, VBE_INFO >> 4
    mov es, bx
    mov si, [es:0x0E] ; Offset for VideoModePtr
    mov ds, [es:0x10] ; Segment for VideoModePtr

    ; Now we can iterate like a string to find the best video mode.
    ; Simple algorithm, either select the desired mode if supported or pick the last one in the table.
    ; Clear bp
    xor bp, bp
next_v_mode:
    lodsw ; AX becomes the word from ptr DS:SI
    cmp ax, 0xffff ; End search
    je end_v_enum ; Zero modes found, terminate

    mov bx, ax ; Save it to bx

    ; Fetch information about each mode
    mov ax, MODE_INFO >> 4
    mov es, ax
    xor di, di

    mov ax, 0x4f01 ; Code for fetching mode info
    mov cx, bx ; Reload word for mode
    int 0x10

    cmp al, 0x4f ; Check if this mode exists
    jne next_v_mode

    ; Is it usable?
    mov ax, [es:0x00] ; Whatever is in this spot gets loaded. If the first bit is comfirmed, it is.
    test ax, 0x0001 ; Supported mode?
    jz next_v_mode
    test ax, 0x0010 ; Is it a video mode or a text-based mode? (ONLY allow video mode!! anything else fucks with the system..)
    jz next_v_mode
    test ax, 0x0080 ; Linear framebuffer?
    jz next_v_mode ; Unsupported

    mov bp, bx

    ; Is it desired?
    cmp bx, VIDEO_DESIRED
    mov bx, cx
    je submit_v_mode

    ; Supported but not desired
    jmp next_v_mode
end_v_enum:
    ; If no video modes at all, and DX is empty, terminate
    test bp, bp
    jz mbr_err

    ; Otherwise, copy back to CX
    mov cx, bp

; CX will be the video mode used during boot.
submit_v_mode:
    ; Set VESA video mode.
    ; The mode information will be saved to 0x6200 (or [MODE_INFO])
    mov ax, 0x4F02
    mov bx, cx ; Copy the mode
    int 0x10

    ; Clear interrupts, clear ds before loading GDT
    cli
    xor ax, ax
    mov ds, ax
    lgdt [gdt_desc]

    ; Return es:bx to 0 before continuing..
    mov ax, 0
    mov es, ax

    ; Transition to 32-bit protected mode by asserting cr0 bit 1
    mov eax, cr0
    or eax, 1
    and eax, 0x7FFFFFFF ; Disable paging
    mov cr0, eax

    ; Long-jump to the start of the kernel.

    jmp code_seg:PMODE_Start

    jmp $ ; Halt program if something goes very wrong..

[bits 32]
PMODE_Start:
    ; Point registers towards our new GDT. Otherwise, reads and writes go to the wrong places.
    mov ax, data_seg
    mov ds, ax
    mov ss, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    mov esp, SP_TOP

    jmp code_seg:KERNEL

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

mbr_err:
    printf_rmode err_str
    jmp $

; Load the GDT into the data section.
%include "Source/Bootload/gdt.s"

; Do not declare as .data, or it will break the boot signature!

test_str:
    db "Booting...",10,13,0

err_str:
    db "[!] Error occured while booting",10,13,0

; Magic boot signature at the end
times 510-($-$$) db 0
dw 0xaa55