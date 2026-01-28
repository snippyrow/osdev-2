#include "stdint.h"
#include "sys.h"
#include "vga.h"

extern "C" void _start() {
    vesa_syscall_return info;
    uint32_t test = call(0x16, (uint32_t)&info, 0, 0); // Get VESA info from kernel.
    
    uint8_t *vga_buffer = (uint8_t*)info.vga_buffer;

    // Try to draw something
    for (uint16_t i = 0; i < 1000; i++) {
        //((uint8_t *)test)[i] = 0xfe;
        vga_buffer[i] = 0xfe;
    }
    return;
}