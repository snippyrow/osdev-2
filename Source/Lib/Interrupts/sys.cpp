#include "sys.h"
#include "stdint.h"
#include "VESA/vga.h"
#include "stdio.h"

struct interrupt_frame {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp_dummy;
    uint32_t ebx, edx, ecx, eax;
    uint32_t int_no, err_code;
    uint32_t eip, cs, eflags, useresp, ss;
};

uint8_t test = 0;
// Generic system call function.
// When 0x80 is invoked by a program, it goes here.
void syscall_stub(interrupt_frame_t *frame) {
    uint32_t sysno = frame -> eax; // Syscall interrupt number is stored in EAX.
    uint32_t a0 = frame -> ebx; // First arg
    uint32_t a1 = frame -> ecx; // Second arg
    uint32_t a2 = frame -> edx; // Third arg

    uint32_t ret = 0;

    // Switch between different system calls.
    // 0x00: NOP
    // 0x16: Get Display information. Returns a pointer to a struct with the VGA Buffer, work buffer, size etc. Must be free'd at the end of the program.
    // 0x18: ATA LBA Read from location (LBA start, #sectors, buffer ptr.)
    // 0x19: ATA LBA Write to location (LBA start, #sectors, buffer ptr.)
    // 0x1A: MALLOC (#blocks), returns ptr.
    // 0x1B: FREE (#blocks, ptr)
    // 0x20: KEYBOARD HOOK (fn. ptr), return event index
    // 0x21: KEYBOARD UNHOOK (event index)
    // 0x40: FAT32 READ (start cluster, buffer, max bytes)
    // 0x41: FAT32 WRITE (start cluster, buffer, max bytes)
    // 0x42: FAT32 CREATE (subdirectory descriptor, dirEntry ptr.)
    // 0x43: FAT32 REMOVE (subdirectory descriptor, file cluster)

    switch (sysno) {
        case 0:
            ret = a0 + a1 + a2;  // example
            break;
        case 0x16: {
            // Find display information.
            // (1) VGA BUFFER
            // (2) WORK BUFFER
            // (3) BPP
            // (4) WIDTH
            // (5) HEIGHT
            
            // Allocate an area of suitable size.
            // FUTURE: REPLACE "1" WITH SUITABLE MATH!!
            uint32_t *rawptr = kmalloc(1);
            struct vesa_syscall_return *vesa_return = (struct vesa_syscall_return *)(uint32_t)(rawptr);

            vesa_return -> vga_buffer = (uint32_t*)VGA_FBUFF;
            vesa_return -> work_buffer = (uint32_t*)WIN_FBUFF;
            vesa_return -> bpp = WIN_BPP;
            vesa_return -> width = WIN_WIDTH;
            vesa_return -> height = WIN_HEIGHT;

            ret = (uint32_t)rawptr;

            break;
        }

        default:
            break;
    }
    frame -> eax = ret;  // return value
    return;
}