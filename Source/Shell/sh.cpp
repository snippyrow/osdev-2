#include "sh.h"
#include "File/fat32.h"
#include "stdint.h"
#include "stdio.h"
#include "VESA/vga.h"
#include "Interrupts/idt.h"

uint8_t *font_ptr;
// char shell_mem[640/8][480/16];
uint16_t cur_x = 0;
uint16_t cur_y = 0;

void putchar(char c, uint16_t x, uint16_t y, uint32_t color) {
    const uint8_t *glyph = &font_ptr[c << 4];

    for (uint32_t b = 0; b < 128; b++) {

        if (glyph[b >> 3] & (1u << (b & 7))) {
            uint16_t px = (b % 8) + x;
            uint16_t py = (b >> 3) + y;

            uint32_t offset = (py * WIN_WIDTH + px) * 3;

            VGA_FBUFF[offset] = 0xff;
            VGA_FBUFF[offset + 1] = 0xff;
            VGA_FBUFF[offset + 2] = 0xff;
        }
    }
    return;
}

uint16_t z = 0;

void kbd_test(uint16_t scancode) {
    putchar(keymap[scancode], cur_x * 8, cur_y * 16, 0);
    cur_x++;
    if (cur_x > WIN_WIDTH / 8) {
        cur_x = 0;
        cur_y++;
    }
    return;
}

// Shell handler. In future, nove to a userspace program and replace all the dependencies with kernel calls.
void sh_start() {
    // Load the font bitmap
    uint32_t sector_begin = find(2, "FONT    BIN");
    if (sector_begin == 0) {
        return;
    }

    // Allocate a spot to load file
    uint32_t *fbuff = kmalloc(9);
    if ((uint32_t)fbuff == 0) {
        return; // No memory left
    }
    font_ptr = (uint8_t*)fbuff; // Assign pointer
    int read_status = read(sector_begin, font_ptr, 4608);

    if (read_status == -1) {
        return;
    }

    kbd_int_connect((uint32_t)kbd_test);

    putchar('Z',200,200,0);

    return;
}