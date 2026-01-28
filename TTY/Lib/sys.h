#ifndef SYSCALLS_H
#define SYSCALLS_H

#include "stdint.h"

uint32_t call(uint32_t sysno, uint32_t a0 = 0, uint32_t a1 = 0, uint32_t a2 = 0);

#endif