#ifndef SYST_H
#define SYST_H

#include "stdint.h"


typedef struct interrupt_frame interrupt_frame_t;

uint32_t syscall_stub(uint32_t sysno, uint32_t a0, uint32_t a1, uint32_t a2);

#endif