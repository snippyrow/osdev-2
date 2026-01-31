#ifndef STANDARDIO
#define STANDARDIO

#include "stdint.h"

#define low_16(address) (short)((address) & 0xFFFF)
#define high_16(address) (short)(((address) >> 16) & 0xFFFF)
#define NULL ((void*)0)
typedef void (*fn_ptr)();

#define MEM_BLOCK_SIZE 512 // Size of allocated memory blocks in bytes
#define BLOCK_NUM 256 // Number of blocks to use
#define TABLE_SIZE (BLOCK_NUM * 8) // Length in bytes for faster compiling

extern uint8_t mem_tracker[TABLE_SIZE]; // Bitmap

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outl(uint16_t port, uint32_t value);

uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);

void insw(uint16_t port, void* buffer, uint32_t count);

void iret();

void sti();
void cli();

void memcpy(void *dest, const void *source, uint32_t nbytes);

// Allocate memory from the kernel space. When a system or user program requests memory, it is given in 512-byte chunks.
// Variable "size_t" states how many blocks of memory are needed, and each program should manage the use internally.
// For freeing up blocks of memory, the pointer for the block is passed back in for ease of use and it all gets cleared at once.
// If something fast needs to happen (50-character string operation) a full block can be used and given up later without much issue.

// Alternatively, another mode could be configured to manually set kernel block size.
uint32_t* kmalloc(uint32_t size_t);

#endif