#include "ls.h"
#include "vga.h"
#include "sys.h"

// I'm just gonna load the boot sector while skipping the kernel. Probably should add a kernel option for this.
struct fat32_bootsector* fat32_info = (struct fat32_bootsector*)0x7c00;

int c_list_files(void) {
    int numbers[2] = {69, 420};
    // Receieve the current directory for the shell into a valid file descriptor, then read

    // Allocate enough space for 1024 bytes (or 32 files).
    // Specify if there are more hidden files. No way to see directroy size.

    // Get memory
    void *ptr = (void*)k_call(0x1a, 2);
    if (!ptr) {
        // Allocation failure, terminate with an error
        return -1;
    }

    // Make a directory before reading
    fat32_dirEntry *dir = (fat32_dirEntry *)ptr;

    // Read files
    int read_err = k_call(0x40, shell_cd, (uint32_t)ptr, 1024);
    if (read_err) {
        // Read error, FREE MEMORY WHEN IMPLEMENTED!!!!***
        return -1;
    }

    tty_printf("\n\rFILE NAME    FILE SIZE\n\n\r");
    // Iterate directory struct until end or EOD object.
    for (uint16_t i = 0; i < 32; i++) {
        if (!dir[i].fname[0]) break; // End of directory, no file here!
        if (dir[i].attributes & 0x0F) continue; // Hidden items
        // Print file name
        bool space = false;
        uint8_t c = 0; // Number of characters printed for UI format

        if (dir[i].attributes & 0x10) {
            // Is a subdirectory
            tty_printf("./");
            for (uint8_t j = 0; j < 11; j++) {
                if (dir[i].fname[j] == ' ') continue;
                tty_putchar(dir[i].fname[j]);
            }
            tty_printf("\n\r");
        } else {
            // Is a file
            for (uint8_t j = 0; j < 11; j++) {
                if (dir[i].fname[j] == ' ') {
                    if (!space) {
                        tty_putchar('.');
                        space = true;
                    }
                    continue;
                }
                if (j == 7 && !space) {
                    tty_putchar('.');
                    space = true;
                }
                tty_putchar(dir[i].fname[j]);
                c++;
            }
            for (uint8_t j = 0; j < 12 - c; j++) {
                tty_putchar(' ');
            }
            int params[1] = {dir[i].size};
            tty_printf("%d B\n\r",params);
        }
    }

    
    return 0;
}