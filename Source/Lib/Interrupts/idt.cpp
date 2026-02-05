#include "idt.h"
#include "sys.h"
#include "stdio.h"
#include "stdint.h"

// Single definition for structures
struct idt_gate idt[256];
struct idt_reg idt_desc;

// Generic IDT handler for testing purposes. Simply fire EOI for end of interrupt.
void idt_ghandler() {
    outb(0x20, 0x20); // EOI
    return;
}

// Define for low and high words of the base address
void set_idt_gate(uint8_t gate, uint32_t hook) {
    idt[gate].offset_lo = (uint16_t)(hook & 0xffff);
    idt[gate].offset_hi = (uint16_t)((hook >> 16) & 0xffff);
    idt[gate].selector = 0x8;
    idt[gate].flags = 0x8e; // flags always the same
    idt[gate].always0 = 0;
}

// Keyboard interrupt handler. Loop though all functions in the table, as demanded by certain programs.
// Each function should take one uint16_t for a chacter (and control stuff) and return as VOID, before terminating the interrupt.
// Allow 256 functions to be used.

// List of all functions that allow programs to receive events
kbd_hook_t kbd_hooks[256] = {0};

void kbd_int_handle() {
    uint8_t scancode = inb(0x60);
    uint16_t code = x86_keycodes[scancode]; // Map to a real scancode
    // These codes seem to break the shift keys, keep them unused for now.
    if (!code) return;
    for (uint16_t j = 0; j < 256; j++) {
        if ((uint32_t)kbd_hooks[j] == 0) continue;
        kbd_hooks[j](scancode); // Call function if it does exist
    }
    return;
}

// Connect code to the keyboard handler in order to receive events
// Return -1 if error, anything else is the index used for de-registering the handler.
int kbd_int_connect(uint32_t function_ptr) {
    kbd_hook_t fn = (kbd_hook_t)(void *)function_ptr;
    for (uint8_t j = 0; j < 256; j++) {
        // Check if the space is unoccupied by anything else.
        if (kbd_hooks[j]) continue;
        kbd_hooks[j] = fn;
        return j;
    }
    return -1;
}

// Add required fields
void idt_install() {

    //set_idt_gate(0,(uint32_t)div0);
    //set_idt_gate(8,(uint32_t)doublefault);

    // https://wiki.osdev.org/Interrupts
    set_idt_gate(0x80, (uint32_t)isr80_stub);
    set_idt_gate(33, (uint32_t)kbd_int_stub);

    // Re-map the master & slave PIC. How does it work? 
    outb(0x20, 0x11);
    outb(0xA0, 0x11);

    // ICW2
    outb(0x21, 0x20);
    outb(0xA1, 0x28);

    // ICW3
    outb(0x21, 0x04);
    outb(0xA1, 0x02);

    // ICW4
    outb(0x21, 0x01);
    outb(0xA1, 0x01);

    // OCW1
    outb(0x21, 0x0);
    outb(0xA1, 0x0);

    outb(0x21, 0b11111001); // only unmask PIT, Cascade PIC and keyboard (IRQ0 and IRQ1 and IRQ2)
    outb(0xA1, 0b11111111); // Only unmask PS/2 mouse


    idt_desc.base = (uint32_t) &idt;
    idt_desc.limit = 0xff * sizeof(idt_gate) - 1;
    asm volatile("lidt (%0)" : : "r" (&idt_desc));
}