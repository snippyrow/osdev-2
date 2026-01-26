#include "VESA/vga.h"
#include "stdint.h"
#include "stdio.h"
#include "Disk/ata.h"
#include "Interrupts/idt.h"
#include "Interrupts/pit.h"

#include "File/fat32.h"

uint8_t a = 0;

uint32_t syscall3(uint32_t sysno, uint32_t a0, uint32_t a1, uint32_t a2) {
    uint32_t ret;

    __asm__ volatile (
        "int $0x80"
        : "=a"(ret)                 // return value from EAX
        : "a"(sysno),               // EAX
          "b"(a0),                  // EBX
          "c"(a1),                  // ECX
          "d"(a2)                   // EDX
        : "memory"
    );

    return ret;
}

void fail() {
    a = a / 0;
}

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

    uint32_t sector_begin = find(2, "TTY     BIN");
    if (sector_begin == 0) {
        fail();
    }

    // Read the file TTY.BIN
    // Request two blocks (for now)
    //uint32_t *fbuff = kmalloc(2);
    int read_status = read(sector_begin, (uint8_t *)0x1000000, 100);
    //uint32_t sec2_begin = find(2, "FINDER2 TXT");
    //read(find(2, "OSYSTEM BIN"), VGA_FBUFF, 1000); // test

    //if (sec2_begin == 0) {
        //fail();
    //}
    //int stat = read(sec2_begin, VGA_FBUFF, 2048); // test
    if (read_status == -1) {
        fail();
    }

    //ata_lba_read(8150, 1, (uint32_t)VGA_FBUFF);

    // Call the entry function
    struct fat32_executable_header* f_headers = (struct fat32_executable_header*)0x1000000;
    func_t entry = (func_t)(f_headers -> entry + 0x1000000);
    entry();

    vga_fillrect(50, 50, 100, 100, 0x00FF0000);
    sti();

    while (1) {
        a = a + 1;
    }
}