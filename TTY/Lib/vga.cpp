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

void tty_putchar(uint8_t c) {
    putchar(c, cur_x * 8, cur_y * 16, 0xffffff00);
    cur_x++;
    if (cur_x > kernel_video.width / 8 - 1) {
        cur_x = 0;
        cur_y++;
    }
    putchar('\xdb', cur_x * 8, cur_y * 16, 0xffffffff);
    return;
}

void tty_printd(int num) {
    int n = num; // Copy for manipulation
    uint32_t rev = 0;

    if (!n) {
        // Very inefficient doing things recursively, potential for a stack overflow.
        // Built-in commands shouldn't worry though.
        tty_putchar('0'); // Print zero if number zero
        return;
    } else if (n < 0) {
        tty_putchar('-'); // Start by printing the negative sign (-0 is possible)
        n = -n; // Invert
    }
    while (n) { // Reverse digits to print most significant digit first
        rev = rev * 10 + (n % 10);
        n /= 10;
    }
    while (rev) {
        tty_putchar((rev % 10) + '0');
        rev /= 10;
    }
    // Print the least significant zero
    if (!(num % 10)) {
        tty_putchar('0');
    }

    return;
}

void tty_printf(char *string, int *options) {
    uint8_t op = 0;
    for (uint16_t i = 0; string[i]; i++) {
        switch (string[i]) {
            case ('\r'): {
                putchar('\xdb', cur_x * 8, cur_y * 16, 0);
                cur_x = 0;
                break;
            }
            case ('\n'): {
                putchar('\xdb', cur_x * 8, cur_y * 16, 0);
                cur_y++;
                break;
            }
            case ('\t'): {
                putchar('\xdb', cur_x * 8, cur_y * 16, 0);
                cur_x+=4;
                break;
            }
            // Make sure that formatting allows a % to be printed.
            case ('\\'): {
                if (string[i+1] == '%') {
                    putchar('%', cur_x * 8, cur_y * 16, 0xffffff00);
                    i++;
                    cur_x++;
                }
                break;
            }
            // My attempt at formatting for numbers.
            case ('%'): {
                if (string[i+1] == 'd') { // Format for decimal
                    tty_printd(options[op]);
                }
                op++;
                i++;
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