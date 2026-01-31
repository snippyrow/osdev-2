#ifndef SYST_H
#define SYST_H

#include "stdint.h"

// First things I push, then CPU pushed items.
typedef struct interrupt_frame {
    // Pushed stuff
    uint32_t eax; // Sysno
    uint32_t ebx;
    uint32_t ecx;
    uint32_t edx;

    // Automatically pushed by CPU
    uint32_t eip;
    uint32_t cs;
    uint32_t eflags;
} __attribute__((packed)) interrupt_frame_t;

extern "C" uint32_t syscall_handle(interrupt_frame_t *frame);

#endif