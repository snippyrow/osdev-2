#ifndef USRVGA_H
#define USRVGA_H

#include "stdint.h"

extern struct vesa_syscall_return kernel_video;

struct vesa_syscall_return {
    uint8_t *vga_buffer;
    uint8_t *work_buffer;
    uint8_t bpp;
    uint16_t width;
    uint16_t height;
} __attribute__ ((packed));

void vga_fetch();

void putchar(char c, uint16_t x, uint16_t y, uint32_t color);

#endif