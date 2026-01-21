#include "vga.h"
#include "stdint.h"
#include "Interrupts/idt.h"

uint8_t a = 0;

extern "C" void kmain() {
    vga_init();
    idt_install();
    uint8_t* fb = WIN_FBUFF;
    /* Set pixel (0,0) to red */
    for (uint8_t x=0;x<64;x++) {
        fb[x*3] = 0x00;  // Blue
        fb[x*3 + 1] = 0x00;  // Green
        fb[x*3 + 2] = 0xFF;  // Red
    }
    while (1) {
        a = a + 1;
    }
}