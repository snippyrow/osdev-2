#include "stdint.h"
#include "sys.h"
#include "vga.h"

/*
Basic shell system to work with the lowest levels of the kernel.
Includes options for running some basic utility commands, as well as support for executing programs.
*/

uint8_t *font_ptr; // Pointer to load file into
char *scancode_map_ptr;
char *shell_cache_memory;

extern "C" void _start() {
    const char FONTNM[12] = "FONT    BIN";
    vga_fetch();

    // Try to read directory and search for file
    uint8_t *dir_ptr = (uint8_t*)malloc(1); // Allocate two blocks for now
    if ((uint32_t)dir_ptr == 0) {
        // Out of memory, fail
        return;
    }

    // Try to read the root directory for "FONT.BIN"
    int read_err = fat32_fs_read(2, dir_ptr, 512);
    if (read_err) {
        // bad read
        return;
    }

    // Can't find file (?) Data is being loaded correctly. Optimization seems to break things.
    fat32_dirEntry *dir = (fat32_dirEntry *)dir_ptr;
    uint32_t file_desc = 0;
    uint32_t font_size;
    uint8_t* vga_buffer = kernel_video.vga_buffer;

    for (uint8_t i = 0; i < 16; i++) {
            // First check if the thing is a file. Then check if the strings match.
        uint8_t strmatch = 0; // If both strings match all the way through, strmatch should be equal to 0
        for (uint8_t j = 0; j < 11; j++) {
            strmatch |= (dir[i].fname[j] ^ FONTNM[j]);
        }
        if (!strmatch && !(dir[i].attributes & 0x10)) { // Do strings match, and is it NOT a directory?
            file_desc = (dir[i].cluster_high << 16) + dir[i].cluster_low;
            font_size = dir[i].size;
            break;
        }
    }

    if (file_desc == 0) {
        return;
    }

    // Read the file into malloc space.
    font_ptr = (uint8_t*)malloc((font_size / 512) + 1);
    fat32_fs_read(file_desc, font_ptr, font_size);

    // Allocate a spot for a shell
    shell_cache_memory = (char*)malloc(1);
    scancode_map_ptr = (char *)keymap;

    // Clear the screen to black before beginning
    for (uint16_t x = 0; x < kernel_video.width * 3; x++) {
        for (uint16_t y = 0; y < kernel_video.height; y++) {
            vga_buffer[x + (y * kernel_video.width * 3)] = 0x00;
        }
    }

    // Hook keyboard
    k_call(0x20, (uint32_t)kbd_test, 0, 0);

    // Fast screen clear
    screen_set_bw(0);

    // Print splash & place first cursor
    tty_println("Welcome to the kernel!\n\rI have nothing else to say.\n\r");

    // Try to draw something
    for (uint32_t i = 0; i < (kernel_video.width * 10 * 3); i++) {
        //vga_buffer[i + (kernel_video.width * 10 * 3)] = i % 256;
    }

    for (;;);

    return;
}