#ifndef IDT_H
#define IDT_H

#include "stdint.h"

// Define external functions from interrupt assembly wrappers
extern "C" uint32_t isr80_stub();
extern "C" void kbd_int_stub();

extern "C" void kbd_int_handle();

// Serves to go into arrays for keyboard/mice/other input devices. Take a single word as an argument.
typedef void (*kbd_hook_t)(uint16_t scancode);

int kbd_int_connect(uint32_t function_ptr);

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

// Keymap for keyboard
const uint16_t x86_keycodes[256] =
	{ 0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14, 15,
	 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31,
	 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47,
	 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63,
	 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79,
	 80, 81, 82, 83, 84,118, 86, 87, 88,115,120,119,121,112,123, 92,
	284,285,309,  0,312, 91,327,328,329,331,333,335,336,337,338,339,
	367,288,302,304,350, 89,334,326,267,126,268,269,125,347,348,349,
	360,261,262,263,268,376,100,101,321,316,373,286,289,102,351,355,
	103,104,105,275,287,279,258,106,274,107,294,364,358,363,362,361,
	291,108,381,281,290,272,292,305,280, 99,112,257,306,359,113,114,
	264,117,271,374,379,265,266, 93, 94, 95, 85,259,375,260, 90,116,
	377,109,111,277,278,282,283,295,296,297,299,300,301,293,303,307,
	308,310,313,314,315,317,318,319,320,357,322,323,324,325,276,330,
	332,340,365,342,343,344,345,346,356,270,341,368,369,370,371,372 };

#endif