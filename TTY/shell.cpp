#include "stdint.h"
#include "sys.h"
#include "vga.h"

extern "C" void _start() {
    const char FONTNM[13] = "FONT    BIN";
    vga_fetch();

    // Try to read directory and search for file
    uint32_t fbuff_ptr = malloc(8); // Allocate two blocks for now
    if (fbuff_ptr == 0) {
        // Out of memory, fail
        return;
    }

    // Try to read the root directory for "FONT.BIN"
    int read_err = fat32_fs_read(2, (uint8_t*)fbuff_ptr, 1000);
    if (read_err) {
        // bad read
        return;
    }

    
    //fat32_dirEntry *dir = (fat32_dirEntry *)fbuff_ptr;
    //uint32_t file_desc = 0;
    uint8_t* vga_buffer = kernel_video.vga_buffer;



    //if (file_desc == 0) {
        //return;
    //}

    // Try to draw something
    for (uint32_t i = 0; i < (kernel_video.width * 10 * 3); i++) {
        //((uint8_t *)test)[i] = 0xfe;
        vga_buffer[i + (kernel_video.width * 10 * 3)] = i % 256;
    }
    return;
}