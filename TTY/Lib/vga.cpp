#include "vga.h"
#include "sys.h"
#include "stdint.h"

struct vesa_syscall_return kernel_video;

// Collect VGA video information from the kernel, put them into the struct.
void vga_fetch() {
    k_call(0x16, (uint32_t)&kernel_video, 0, 0);
    return;
}