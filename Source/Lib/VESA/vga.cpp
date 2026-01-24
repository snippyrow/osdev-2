#include "vga.h"
#include "stdio.h"

// Define VESA/VGA characteristics
struct vbe_mode_info_structure* vbe_info = (struct vbe_mode_info_structure*)0x6200;

uint16_t WIN_WIDTH  = 0;
uint16_t WIN_HEIGHT = 0;
uint8_t  WIN_BPP    = 0;
uint8_t* WIN_FBUFF  = 0;
uint8_t* VGA_FBUFF  = 0;

// ARGB
void vga_fillrect(uint16_t x, uint16_t y, uint16_t sx, uint16_t sy, uint32_t color, bool mirror) {
    uint8_t r = (color >> 24) & 0xff;
    uint8_t g = (color >> 16) & 0xff;
    uint8_t b = (color >> 8)  & 0xff;

    for (uint16_t py = y; py < y + sy; py++) {
        for (uint16_t px = x; px < x + sx; px++) {
            uint32_t offset = (py * WIN_WIDTH + px) * 3;
            WIN_FBUFF[offset + 0] = b;
            WIN_FBUFF[offset + 1] = g;
            WIN_FBUFF[offset + 2] = r;

            if (mirror) {
                VGA_FBUFF[offset + 0] = b;
                VGA_FBUFF[offset + 1] = g;
                VGA_FBUFF[offset + 2] = r;
            }

            // This does not really take advantage of the double-buffer system, so just copy the image to both
        }
    }
    return;
}

void vga_init() {
    // The WIN_FBUFF is NOT the real VGA framebuffer, but instead a work buffer.
    // That would be: (uint8_t *)vbe_info -> framebuffer
    // WIN_FBUFF = (uint8_t *)vbe_info -> framebuffer;
    WIN_WIDTH = vbe_info -> width;
    WIN_HEIGHT = vbe_info -> height;
    WIN_BPP = vbe_info -> bpp;
    // Do some math. Each block is 512 bytes, round up. BPP is usually 24, so divide by eight to get bytes per pixel.
    // If using monochrome (one bit per pixel) some floating math is used.
    uint32_t buffer_blocks = (WIN_WIDTH * WIN_HEIGHT) * (WIN_BPP / 8.0f) / 512;
    uint32_t* buffer = kmalloc(buffer_blocks);
    WIN_FBUFF = (uint8_t *)buffer;
    VGA_FBUFF = (uint8_t *)vbe_info -> framebuffer;
    return;
}

// VGA/VESA will work as a double-buffered system, handled automatically by the video system.
// After each draw commad, if specified it will perform a fast memory copy to the display buffer.
// For example, if drawing a circle it would begin plotting the circle, fill it then perform a memory copy on all the modified pixels.
// If many operations are happening, user can specify to not copy and trigger it manually later.

