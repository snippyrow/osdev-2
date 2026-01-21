#include "stdio.h"
#include "stdint.h"

// Output value to I/O port, 8 bits
void outb(uint16_t port, uint8_t value){
    asm volatile("outb %1, %0" : : "dN" (port), "a" (value));
}

void outw(uint16_t port, uint16_t value) {
    asm volatile("outw %1, %0" : : "dN" (port), "a" (value));
}

void iret() {
    asm volatile("iret");
}

// Get input for a specified I/O port, 8 bits
unsigned char inb(uint16_t port) {
    unsigned char result;
    __asm__("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

unsigned short inw(uint16_t port) {
    unsigned short ret;
    asm volatile ("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}

void outl(uint16_t port, uint32_t value) {
    asm volatile("outl %0, %1" : : "a"(value), "Nd"(port));
}

// Read from PCI configuration data port
uint32_t inl(uint16_t port) {
    uint32_t value;
    asm volatile("inl %1, %0" : "=a"(value) : "Nd"(port));
    return value;
}