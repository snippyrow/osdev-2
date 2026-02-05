#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "stdint.h"


int fat32_fs_read(uint32_t descriptor, uint8_t *buffer, uint32_t max_read);

uint32_t malloc(uint32_t num_blocks);

extern "C" uint32_t k_call(uint32_t sysno, uint32_t a0 = 0, uint32_t a1 = 0, uint32_t a2 = 0);

// Memory for anything typed into the prompt by a user before entering.
extern char *shell_cache_memory;

extern uint16_t cur_x;
extern uint16_t cur_y;

// This links to the pointer for the keymap we want to use. When pressing shift, it moves.
// Not working any different, just more scale-able and branchless. Defined at start.
extern char *scancode_map_ptr;

void kbd_test(uint16_t scancode);

struct fat32_dirEntry {
    uint8_t fname[11]; // Filename and extension (padded with spaces)
    uint8_t attributes; // See *
    uint8_t reserved;
    uint8_t create_time_fine; // Use 10mz units.
    uint16_t create_time; // See **
    uint16_t create_date; //  See ***
    uint16_t access_date;
    uint16_t cluster_high; // High two bytes of the first cluster index in the FAT.
    uint16_t modified_time;
    uint16_t modified_date;
    uint16_t cluster_low; // Low two bytes of cluster in the FAT. Also works for FAT12 & FAT16.
    uint32_t size; //Size in bytes (maximum 4GB!)
} __attribute__((packed));

extern uint8_t *font_ptr;

const char keymap[] = {
    '?', '?', '1', '2', '3', '4', '5',
    '6', '7', '8', '9', '0', '-', '=',
    '?', '?', 'q', 'w', 'e', 'r', 't',
    'y', 'u', 'i', 'o', 'p', '[', ']',
    '?', '?', 'a', 's', 'd', 'f', 'g',
    'h', 'j', 'k', 'l', ';', '\\', '`',
    '?', '\\', 'z', 'x', 'c', 'v', 'b',
    'n', 'm', ',', '.', '/', '?', '?',
    '?', ' '
};

const char keymap_shift[] = {
    '?', '?', '!', '@', '#', '$', '%',
    '^', '&', '*', '(', ')', '_', '+',
    '?', '?', 'Q', 'W', 'E', 'R', 'T',
    'Y', 'U', 'I', 'O', 'P', '[', ']',
    '?', '?', 'A', 'S', 'D', 'F', 'G',
    'H', 'J', 'K', 'L', ':', '\\', '~',
    '?', '\\', 'Z', 'X', 'C', 'V', 'B',
    'N', 'M', '<', '>', '?', '?', '?',
    '?', ' '
};

#endif