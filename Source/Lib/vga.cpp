#include <vga.h>

// Define VESA/VGA characteristics
struct vbe_mode_info_structure* vbe_info = (struct vbe_mode_info_structure*)0x6200;

uint16_t WIN_WIDTH  = 0;
uint16_t WIN_HEIGHT = 0;
uint8_t  WIN_BPP    = 0;
uint8_t* WIN_FBUFF  = 0;

void vga_fillrect(uint16_t x, uint16_t y, uint16_t sx, uint16_t sy, uint32_t color) {
    return;
}

void vga_init() {
    WIN_FBUFF = (uint8_t *)vbe_info -> framebuffer;
    WIN_WIDTH = vbe_info -> width;
    WIN_HEIGHT = vbe_info -> height;
    WIN_BPP = vbe_info -> bpp;
    return;
}