#include "stdint.h"
#include "sys.h"

uint8_t a = 0;

void fail() {
    a = a / 0;
}

int fat32_fs_read(uint32_t descriptor, uint8_t *buffer, uint32_t max_read) {
    uint32_t err = k_call(0x40, descriptor, (uint32_t)buffer, max_read);
    if (err) {
        return -1;
    }
    return 0;
}

uint32_t malloc(uint32_t num_blocks) {
    uint32_t ptr_ret = k_call(0x1a, num_blocks, 0, 0);
    return ptr_ret;
}