#include "vga.h"
#include "sys.h"
#include "stdint.h"

struct vesa_syscall_return kernel_video;

// Collect VGA video information from the kernel, put them into the struct.
void vga_fetch() {
    k_call(0x16, (uint32_t)&kernel_video, 0, 0);
    return;
}

void putchar(char c, uint16_t x, uint16_t y, uint32_t color) {
    const uint8_t *glyph = &font_ptr[c << 4];

    for (uint32_t b = 0; b < 128; b++) {

        if (glyph[b >> 3] & (1u << (b & 7))) {
            uint16_t px = (b % 8) + x;
            uint16_t py = (b >> 3) + y;

            uint32_t offset = (py * kernel_video.width + px) * 3;

            (kernel_video.vga_buffer)[offset] = 0xff;
            (kernel_video.vga_buffer)[offset + 1] = 0xff;
            (kernel_video.vga_buffer)[offset + 2] = 0xff;
        }
    }
    return;
}