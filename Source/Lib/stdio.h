#ifndef STANDARDIO
#define STANDARDIO

#include "stdint.h"

#define low_16(address) (short)((address) & 0xFFFF)
#define high_16(address) (short)(((address) >> 16) & 0xFFFF)
#define NULL ((void*)0)
typedef void (*fn_ptr)();

void outb(uint16_t port, uint8_t value);
void outw(uint16_t port, uint16_t value);
void outl(uint16_t port, uint32_t value);

uint8_t inb(uint16_t port);
uint16_t inw(uint16_t port);
uint32_t inl(uint16_t port);

void iret();

#endif