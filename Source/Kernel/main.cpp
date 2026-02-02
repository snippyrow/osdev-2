#include "VESA/vga.h"
#include "stdint.h"
#include "stdio.h"
#include "Disk/ata.h"
#include "Interrupts/idt.h"
#include "Interrupts/pit.h"

#include "File/fat32.h"

#include "../Shell/sh.h"

uint8_t a = 0;

void fail() {
    a = a / 0;
}

extern "C" void kmain() {
    vga_init();
    idt_install();

    //ata_init();

    // Test the read filesystem by showing an image
    uint32_t test_sec = find(2, "BIRDS   BMP");
    vga_fillrect(0,0,WIN_WIDTH,50,0xFF000000);
    read(test_sec, (uint8_t *)VGA_FBUFF, 1000000);

    // Call the shell system
    sh_start();

    // Read the first byte in the file buffer
    //if (*((uint8_t*)&fbuff[0]) != 0x55) {
        //fail();
    //}

    // Call the entry function
    //struct fat32_executable_header* f_headers = (struct fat32_executable_header*)(uint32_t)fbuff;
    //func_t entry = (func_t)((uint32_t)fbuff);
    //entry();

    vga_fillrect(50, 50, 100, 100, 0x00FF0000);
    sti();

    for (;;) {
        a = a + 1;
    }
}