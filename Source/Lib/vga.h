#ifndef VGA_H
#define VGA_H

// C/C++ naming magic when using external library headers

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

void vga_fillrect(uint16_t x, uint16_t y, uint16_t sx, uint16_t sy, uint32_t color);
void vga_init();

// VESA video structure. When a resolution from the BIOS list was selected while the MBR was running, the structure was saved into [0x6200].
// This data structure is placed overtop, so that details about the screen can be gained by the kernel.

struct vbe_mode_info_structure {
    uint16_t attributes;
    uint8_t window_a;
    uint8_t window_b;
    uint16_t granularity;
    uint16_t window_size;
    uint16_t segment_a;
    uint16_t segment_b;
    uint32_t win_func_ptr;
    uint16_t pitch;
    uint16_t width;
    uint16_t height;
    uint8_t w_char;
    uint8_t y_char;
    uint8_t planes;
    uint8_t bpp;
    uint8_t banks;
    uint8_t memory_model;
    uint8_t bank_size;
    uint8_t image_pages;
    uint8_t reserved0;

    uint8_t red_mask;
    uint8_t red_position;
    uint8_t green_mask;
    uint8_t green_position;
    uint8_t blue_mask;
    uint8_t blue_position;
    uint8_t reserved_mask;
    uint8_t reserved_position;
    uint8_t direct_color_attributes;

    uint32_t framebuffer;
    uint32_t off_screen_mem_off;
    uint16_t off_screen_mem_size;
    uint8_t reserved[206];
} __attribute__ ((packed));

extern struct vbe_mode_info_structure* vbe_info;

// Data will be saved here when the graphics library is started.
extern uint16_t WIN_WIDTH;
extern uint16_t WIN_HEIGHT;
extern uint8_t WIN_BPP;

extern uint8_t *WIN_FBUFF;

#ifdef __cplusplus
}
#endif

#endif // VGA_H