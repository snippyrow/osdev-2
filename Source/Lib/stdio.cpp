#include "stdio.h"
#include "stdint.h"

// Ability to use the I/O ports from C/C++. 
void outb(uint16_t port, uint8_t value){
    asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
}

void outw(uint16_t port, uint16_t value) {
    asm volatile("outw %1, %0" : : "dN" (port), "a" (value));
}

void outl(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

uint8_t inb(uint16_t port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

uint16_t inw(uint16_t port) {
    unsigned short ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

// Read from PCI configuration data port
uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}

// Where things get really funky, when we input a string from an IO port.
void insw(uint16_t port, void* buffer, uint32_t count) {
    asm volatile (
        "rep insw"
        : "+D"(buffer), "+c"(count)
        : "d"(port)
        : "memory"
    );
}


// Return from interrupt (maybe don't use this.....)
void iret() {
    asm volatile("iret");
}

// Methods for enabling/disabling ALL system interrupts
void sti() {
    asm volatile("sti");
}

void cli() {
    asm volatile("cli");
}

void memcpy(void *dest, const void *source, uint32_t nbytes) {
    uint8_t *d = (uint8_t *)dest; // Treat as byte pointers
    const uint8_t *s = (const uint8_t *)source;
    for (uint32_t i = 0; i < nbytes; i++) {
        d[i] = s[i]; // Copy each byte
    }
}

// For 512-byte blocks, size (is using 1GB RMA) is 2^30 / 512 / 8 ~= 262KB. SImply add that to the MEM_TRACKER offset for the first block.
uint8_t *mem_tracker = (uint8_t *)MEM_TRACKER;

// Return a 32-bit pointer
// Size_t is the number of blocks we need.
uint32_t* kmalloc(uint32_t size_t) {
    // When allocating, go up each byte sequentially.
    uint32_t counted = 0;
    uint32_t* ptr_begin = 0;
    for (uint32_t byte = 0; byte<TABLE_SIZE; byte++) {
        for (uint8_t bit_index = 0; bit_index < 8; bit_index++) {
            unsigned bit = (mem_tracker[byte] >> bit_index) & 1u; // Produce a 1 or 0
            if (!bit) { // If unused
                if (counted == 0) {
                    ptr_begin = (uint32_t*)(MEM_TRACKER + TABLE_SIZE + (byte * MEM_BLOCK_SIZE * 8) + (bit_index * MEM_BLOCK_SIZE)); // Declare the start of the pointer
                }
                counted++; // Increment
                if (counted == size_t) { // Found a continuous string that's long enough.
                    // Mark as allocated.
                    uint32_t start_bit = byte * 8 + bit_index - (size_t - 1);

                    for (uint32_t i = 0; i < size_t; i++) {
                        uint32_t b = start_bit + i;
                        mem_tracker[b / 8] |= (1u << (b % 8));
                    }

                    return ptr_begin;
                }
            } else {
                counted = 0; // Reset counter of found blocks if the stream gets interrupted
            }
        }
    }
    return 0; // Out of memory.
}