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

uint8_t mem_tracker[TABLE_SIZE];

// Return a 32-bit pointer
// Size_t is the number of blocks we need.
uint32_t* kmalloc(uint32_t size_t) {
    if (size_t == 0) {
        return (uint32_t*)0; // Bad argument
    }
    // When allocating, go up each byte sequentially.
    uint32_t counted = 0;
    uint32_t bit_start = 0;

    for (uint32_t bit = 0; bit < (BLOCK_NUM * 8); bit++) {
        if ((mem_tracker[bit >> 3] & (1u << (bit & 7))) == 0) {
            // Found a free block?
            if (counted == 0) {
                bit_start = bit;
            }

            counted++;
            if (counted == size_t) { // Done, begin allocation
                for (uint32_t i = 0; i < size_t; i++) {
                    uint32_t b = bit_start + i;
                    mem_tracker[b >> 3] |= (1u << (b & 7));
                }
                // Return pointer
                return (uint32_t*)(HEAP_START + bit_start * MEM_BLOCK_SIZE);
            }
        } else {
            counted = 0;
            bit_start = 0;
        }
    }
    
    return (uint32_t*)0; // Out of memory.
}

void kfree(void* ptr, uint32_t size_t) {
    if (!ptr || size_t == 0) {
        return;
    }
    // Check MEM_RES_BUFFER if something went wrong!
    uint32_t start = ((uint8_t*)ptr - (uint8_t*)mem_tracker + TABLE_SIZE - HEAP_START) / MEM_BLOCK_SIZE;

    for (uint32_t i = 0; i < size_t; i++) {
        uint32_t b = start + i;
        mem_tracker[b >> 3] &= ~(1u << (b & 7));
    }
}