#ifndef SYST_H
#define SYST_H

#include "stdint.h"


typedef struct interrupt_frame interrupt_frame_t;

void syscall_stub(interrupt_frame_t *frame);

#endif