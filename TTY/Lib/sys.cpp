#include "stdint.h"
#include "sys.h"
#include "vga.h"

// Include the various command addons
#include "../Addons/ls.h"

uint16_t shell_line_length = 0;
uint16_t cur_x = 0;
uint16_t cur_y = 0;

uint32_t shell_cd = 2;

// Create a list of shell commands.
shell_command shell_builtin_commands[] = {
    {"list", c_list_files, 0}
};

// Number of valid commands to check upon base execution
const uint8_t shell_num_builtin = (uint8_t)(sizeof(shell_builtin_commands) / sizeof(shell_builtin_commands[0]));

int fat32_fs_read(uint32_t descriptor, uint8_t *buffer, uint32_t max_read) {
    uint32_t err = k_call(0x40, descriptor, (uint32_t)buffer, max_read);
    if (err) {
        return -1;
    }
    return 0;
}

uint32_t malloc(uint32_t num_blocks) {
    uint32_t ptr_ret = k_call(0x1a, num_blocks, 0, 0);;
    return ptr_ret;
}


/* Compare S1 and S2, returning less than, equal to or
   greater than zero if S1 is lexicographically less than,
   equal to or greater than S2.  */
int strcmp(const char *p1, const char *p2) {
    const uint8_t *s1 = (const uint8_t *) p1;
    const uint8_t *s2 = (const uint8_t *) p2;
    uint8_t c1, c2;

    do {
        c1 = (unsigned char) *s1++;
        c2 = (unsigned char) *s2++;
        if (c1 == '\0')
	        return c1 - c2;
        }
    while (c1 == c2);

    return c1 - c2;
}

void toLower(char *s) {
    while(*s) {
        if(*s >= 'A' && *s <= 'Z') {
            *s = *s + 32;
        }
        s++;
    }
    return;
}

void memset(void *dest, int val, register uint32_t len) {
    uint8_t *ptr = (uint8_t*)dest;
    while (len-- > 0) {
        *ptr++ = val;
    }
    return;
}


void enter_handle() {
    // Loop through the number of valid commands before running through programs.

    // Set the keyboard buffer to lowercase
    bool ranCommand = false;
    toLower(shell_cache_memory);
    for (uint8_t c = 0; c < shell_num_builtin; c++) {
        if (strcmp(shell_cache_memory, shell_builtin_commands[c].cname)) continue; // No match
        // Call the command
        int err = shell_builtin_commands[c].caller();
        ranCommand = true;
        if (err) {
            tty_printf("Command completed with one or more errors. Please check output.\n\r");
            break;
        }
        break;
    }
    if (!ranCommand) {
        tty_printf("Unknown command or method. Please check spelling.\n\r");
    }

    // Set memory to zero
    memset(shell_cache_memory, 0, MAX_INPUT);
    shell_line_length = 0;
    return;
}

// Any scancodes received by the kernel go here. Check for special characters, too.
void kbd_test(uint16_t scancode) {
    switch(scancode) {
        case (0x1c): { // Enter key pressed
            putchar('\xdb', cur_x * 8, cur_y * 16, 0); // Erase cursor
            cur_x = 0;
            cur_y++;
            putchar('\xdb', cur_x * 8, cur_y * 16, 0xffffffff); // Replay cursor
            enter_handle();
            break;
        }
        case (0x0e): { // Backspace
            if (shell_line_length < 1) break;
            putchar('\xdb', cur_x * 8, cur_y * 16, 0); // Black out the current cursor and replace klast character with new cursor
            cur_x--;
            shell_line_length--;
            shell_cache_memory[shell_line_length] = '\0'; // Remove from user field
            putchar('\xdb', cur_x * 8, cur_y * 16, 0xffffffff);

            // Erase from the shell memory
            shell_cache_memory[shell_line_length] = 0;
            break;
        }
        case (0x2a): { // LShift pressed
            scancode_map_ptr = (char *)keymap_shift;
            break;
        }
        case (0x36): { // RShift pressed
            scancode_map_ptr = (char *)keymap_shift;
            break;
        }
        case (0xaa): { // RShift depressed
            scancode_map_ptr = (char *)keymap;
            break;
        }
        case (0xb6): { // RShift depressed
            scancode_map_ptr = (char *)keymap;
            break;
        }

        default: {
            // Check for a printable character
            if (scancode < 0x01 || scancode > 0x39) break;
            if (shell_line_length > MAX_INPUT) break; // If line too long already
            
            putchar(scancode_map_ptr[scancode], cur_x * 8, cur_y * 16, 0xffffffff);
            putchar('\xdb', (cur_x + 1) * 8, cur_y * 16, 0xffffffff); // Allows us to see what we type.
            shell_cache_memory[shell_line_length] = scancode_map_ptr[scancode];
            shell_line_length++;
            cur_x++;
            break;
        }
    }
    if (cur_x > kernel_video.width / 8 - 1) {
        cur_x = 0;
        cur_y++;
    }
    return;
}