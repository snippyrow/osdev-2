#include "stdint.h"
#include "sys.h"
#include "vga.h"

uint16_t shell_line_length = 0;
uint16_t cur_x = 0;
uint16_t cur_y = 0;

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

void enter_handle() {
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
            if (shell_line_length > 128) break; // If line too long already
            
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