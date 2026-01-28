#ifndef USRVGA_H
#define USRVGA_H

#include "stdint.h"

struct vesa_syscall_return {
    uint32_t vga_buffer;
    uint32_t work_buffer;
    uint8_t bpp;
    uint16_t width;
    uint16_t height;
} __attribute__ ((packed));

vesa_syscall_return vesa_return();

#endif