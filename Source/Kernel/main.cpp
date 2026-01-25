#include "VESA/vga.h"
#include "stdint.h"
#include "stdio.h"
#include "Disk/ata.h"
#include "Interrupts/idt.h"
#include "Interrupts/pit.h"

#include "File/fat32.h"

uint8_t a = 0;

extern "C" void kmain() {
    vga_init();
    idt_install();
    uint8_t* fb = WIN_FBUFF;
    /* Set pixel (0,0) to red */
    //ata_init();
    int st = ata_lba_read(5, 4, 0x100000);
    uint8_t test_buffer[512];
    //test_buffer[0] = 'e';
    //int status = read(2, VGA_FBUFF, 100);
    //ata_lba_read(32, 1, (uint32_t)VGA_FBUFF);

    uint32_t sector = find(2, "OSYSTEM BIN");
    if (sector == 0) {
        a = a / 0;
    }

    vga_fillrect(50, 50, 100, 100, 0x00FF0000);

    pit_setfreq(30);
    //sti();

    while (1) {
        a = a + 1;
    }
}