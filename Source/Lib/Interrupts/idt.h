#ifndef IDT_H
#define IDT_H

#include "stdint.h"

// Define an IDT gate. When loaded into the IDT at a specific index, the processor will use this gate to figure out what to do.
// Selector should be 0x8 for CODE, and the address is packed into low and high ends. ONLY WORKS FOR 32-BIT PMODE!!
typedef struct idt_gate {
    uint16_t offset_lo; // Address of function hook, low word
    uint16_t selector;  // Segment selector (0x8 for code seg)
    uint8_t always0;    // Reserved
    uint8_t flags;      // Flags
    uint16_t offset_hi; // Address of function hook, high word
} __attribute__((packed)) idt_gate;

// IDT Register, this is what we give to the processor using the "lidt" instruction. The "limit" is low large our table is in bytes.
// The "base" is where our table is, using a linear address mode. These tell the processor how to use the table.
typedef struct idt_reg {
    uint16_t limit;
    uint32_t base;
} __attribute__((packed)) idt_reg;

// Declare our IDT, and link the BASE here.
extern idt_gate idt[256];

// IDT descriptor, much like the GDT descriptor.
extern idt_reg idt_desc;

// Declare main functions for working with IDT:
void idt_install();

// Generic IDT handler for testing purposes. Simply fire EOI for end of interrupt.
void idt_ghandler();

#endif