#include "vga.h"
#include "sys.h"
#include "stdint.h"

vesa_syscall_return vesa_return() {
    vesa_syscall_return *ret = (vesa_syscall_return *)call(0x16); // Generate a new struct at the pointer returned by syscall.
    return *ret;
}