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

/*
FIX:
At the moment, creating new subdirectories causes it to not be visible at the mountpoint and be deleted.
Unclear if some of the functions are working correctly, should probably check those.
Also fix some of the code to have cleaner/better argument and return types.
*/

extern "C" void kmain() {
    vga_init();
    idt_install();

    //ata_init();

    // Test the read filesystem by showing an image
    //uint32_t test_sec = find(2, "BIRDS   BMP");
    vga_fillrect(0,0,WIN_WIDTH,50,0xFF000000);
    //read(test_sec, (uint8_t *)VGA_FBUFF, 1000000);


    // Make a test directory.
    fat32_dirEntry *testdir = (fat32_dirEntry*)kmalloc(1);
    char teststr[13] = "TESTDIR     ";
    memcpy(testdir -> fname, teststr, 11);
    testdir->attributes = 0x10;
    uint32_t new_s = fat32_mko(2, testdir);
    if (!new_s) {
        fail();
    }

    uint32_t starting_sector = find(2, "TTY     BIN");
    if (starting_sector == 0) {
        fail();
    }

    uint8_t *fbuff = (uint8_t*)(0x300000);
    if ((uint32_t)fbuff == 0) {
        fail();
    }

    int read_err = read(starting_sector, fbuff, 50000);
    if (read_err) {
        fail();
    }

    sti();

    // Call the entry function
    //struct fat32_executable_header* f_headers = (struct fat32_executable_header*)(uint32_t)fbuff;
    func_t entry = (func_t)((uint32_t)(fbuff));
    entry();

    vga_fillrect(50, 50, 100, 100, 0x00FF0000);

    for (;;);
}