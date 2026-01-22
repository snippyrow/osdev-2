#include "VESA/vga.h"
#include "stdint.h"
#include "stdio.h"
#include "Interrupts/idt.h"
#include "Interrupts/pit.h"

uint8_t a = 0;

extern "C" void kmain() {
    vga_init();
    idt_install();
    uint8_t* fb = WIN_FBUFF;
    /* Set pixel (0,0) to red */

    vga_fillrect(50, 50, 100, 100, 0x00FF0000);

    pit_setfreq(30);
    //sti();

    while (1) {
        a = a + 1;
    }
}