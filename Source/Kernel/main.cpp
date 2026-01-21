#include "vga.h"
#include <stdint.h>

uint8_t a = 0;

extern "C" void kmain() {
    vga_init();
    uint8_t* fb = WIN_FBUFF;
    /* Set pixel (0,0) to red */
    fb[0] = 0x00;  // Blue
    fb[1] = 0x00;  // Green
    fb[2] = 0xFF;  // Red
    while (1) {
        a = a + 1;
    }
}