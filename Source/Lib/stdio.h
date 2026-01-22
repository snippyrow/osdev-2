#ifndef STANDARDIO
#define STANDARDIO

#include "stdint.h"

#define low_16(address) (short)((address) & 0xFFFF)
#define high_16(address) (short)(((address) >> 16) & 0xFFFF)
#define NULL ((void*)0)
typedef void (*fn_ptr)();

#define MEM_TRACKER 0x100000 // Address to store the table which contains memory block (malloc) tracking
#define MEM_BLOCK_SIZE 512 // Size of allocated memory blocks in bytes
#define TABLE_SIZE (2^30) / MEM_BLOCK_SIZE / 8 // Length in bytes for faster compiling

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outl(uint16_t port, uint32_t value);

uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);

void iret();

void sti();
void cli();

// Allocate memory from the kernel space. When a system or user program requests memory, it is given in 512-byte chunks.
// Variable "size_t" states how many blocks of memory are needed, and each program should manage the use internally.
// For freeing up blocks of memory, the pointer for the block is passed back in for ease of use and it all gets cleared at once.
// If something fast needs to happen (50-character string operation) a full block can be used and given up later without much issue.

// Alternatively, another mode could be configured to manually set kernel block size.
uint32_t* kmalloc(uint32_t size_t);

// Declare table for blocks (Each byte tracks eight blocks)
extern uint8_t *mem_tracker;

#endif