#include "vga.h"
#include "sys.h"
#include "stdint.h"

struct vesa_syscall_return kernel_video;

// Collect VGA video information from the kernel, put them into the struct.
void vga_fetch() {
    k_call(0x16, (uint32_t)&kernel_video, 0, 0);
    return;
}

// RGBA
void putchar(uint8_t c, uint16_t x, uint16_t y, uint32_t color) {
    const uint8_t *glyph = &font_ptr[c << 4];

    uint8_t r = (color >> 24) & 0xff;
    uint8_t g = (color >> 16) & 0xff;
    uint8_t b = (color >> 8) & 0xff;

    for (uint32_t i = 0; i < 128; i++) {

        uint8_t r = (color >> 24) & 0xff;
        uint8_t g = (color >> 16) & 0xff;
        uint8_t b = (color >> 8) & 0xff;

        if (!(glyph[i >> 3] & (1u << (i & 7)))) {
            r = 0;
            g = 0;
            b = 0;
        }
        uint16_t px = (i % 8) + x;
        uint16_t py = (i >> 3) + y;

        uint32_t offset = (py * kernel_video.width + px) * 3;

        // Write character to both buffers, erase background if not a colroed pixel for simplicity.
        (kernel_video.vga_buffer)[offset] = r;
        (kernel_video.vga_buffer)[offset + 1] = g;
        (kernel_video.vga_buffer)[offset + 2] = b;

        (kernel_video.work_buffer)[offset] = r;
        (kernel_video.work_buffer)[offset + 1] = g;
        (kernel_video.work_buffer)[offset + 2] = b;
    }
    return;
}

void tty_println(char *string) {
    for (uint16_t i = 0; string[i]; i++) {
        switch (string[i]) {
            case ('\r'): {
                cur_x = 0;
                break;
            }
            case ('\n'): {
                cur_y++;
                break;
            }
            case ('\t'): {
                cur_x+=4;
                break;
            }
            default: {
                putchar(string[i], cur_x * 8, cur_y * 16, 0xffffff00);
                cur_x++;
                break;
            }
        }
        if (cur_x > kernel_video.width / 8 - 1) {
            cur_x = 0;
            cur_y++;
        }
    }
    putchar('\xdb', cur_x * 8, cur_y * 16, 0xffffffff);
    return;
}


// Fill both the work buffer and frame buffer with one color (black-white for simplicity and speed)
// All three channels get the same color.
void screen_set_bw(uint8_t color) {
    uint32_t bytes = kernel_video.width * kernel_video.height * 3;
    return;
    // Clear framebuffer
    __asm__ volatile (
        "rep stosb"
        : "+D" (kernel_video.vga_buffer), "+c" (bytes)
        : "a" (color)
        : "memory", "cc"
    );

    // Clear workbuffer
    __asm__ volatile (
        "rep stosb"
        : "+D" (kernel_video.work_buffer), "+c" (bytes)
        : "a" (color)
        : "memory", "cc"
    );
    return;
}