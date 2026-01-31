#include "sys.h"
#include "stdint.h"
#include "VESA/vga.h"
#include "stdio.h"
#include "File/fat32.h"

uint8_t test = 0;
// Generic system call function.
// When 0x80 is invoked by a program, it goes here.
uint32_t syscall_handle(interrupt_frame_t *frame) {
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
        case 0x1a: {
            // Allocate blocks
            // a0: # of blocks to allocate, return start ptr
            uint32_t* ptr = kmalloc(a0);
            ret = (uint32_t)ptr;
            //ret = 0x10000000;
            break;
        }
        case 0x16: {
            // Find display information.
            // (1) VGA BUFFER
            // (2) WORK BUFFER
            // (3) BPP
            // (4) WIDTH
            // (5) HEIGHT
            
            // Allocate an area of suitable size.
            // FUTURE: REPLACE "1" WITH SUITABLE MATH!!
            struct vesa_syscall_return tmp = {
                .vga_buffer = (uint32_t)VGA_FBUFF,
                .work_buffer = (uint32_t)WIN_FBUFF,
                .bpp = WIN_BPP,
                .width = WIN_WIDTH,
                .height = WIN_HEIGHT,
            };

            // Copy to user-defined area of memory.
            memcpy((uint8_t *)a0, &tmp, sizeof(tmp));

            break;
        }
        case 0x40: {
            // a0: Descriptor/parent starting cluster
            // a1: Buffer to read the file into
            // a2: Maximum bytes to read

            // Please implement memory safety later.
            int fat32_status = read(a0, (uint8_t*)a1, a2);
            if (fat32_status) {
                ret = 1; // Return a 1 if error, default return is 0
            }
            break;
        }
        case 0x17:
            vga_fillrect(150, 150, 100, 100, 0x00FFFF00);

            break;
        case 0x18:
            ret = 0x68;
            break;

        default:
            break;
    }
    //frame -> eax = ret;  // return value
    return ret;
}