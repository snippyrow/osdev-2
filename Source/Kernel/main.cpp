#include "VESA/vga.h"
#include "stdint.h"
#include "stdio.h"
#include "Disk/ata.h"
#include "Interrupts/idt.h"
#include "Interrupts/pit.h"

#include "File/fat32.h"

uint8_t a = 0;

void fail() {
    a = a / 0;
}

extern "C" void kmain() {
    vga_init();
    idt_install();
    uint8_t* fb = WIN_FBUFF;

    //ata_init();
    //int st = ata_lba_read(5, 4, 0x100000);

    uint32_t sector_begin = find(2, "TTY     BIN");
    if (sector_begin == 0) {
        fail();
    }

    // Read the file TTY.BIN
    // Request two blocks (for now)
    uint32_t *fbuff = kmalloc(4);
    if ((uint32_t)fbuff == 0) {
        fail(); // No memory left
    }
    int read_status = read(sector_begin, (uint8_t *)fbuff, 2048);

    if (read_status == -1) {
        fail();
    }

    // Read the first byte in the file buffer
    //if (*((uint8_t*)&fbuff[0]) != 0x55) {
        //fail();
    //}

    // Call the entry function
    //struct fat32_executable_header* f_headers = (struct fat32_executable_header*)(uint32_t)fbuff;
    func_t entry = (func_t)((uint32_t)fbuff);
    entry();

    vga_fillrect(50, 50, 100, 100, 0x00FF0000);
    sti();

    while (1) {
        a = a + 1;
    }
}