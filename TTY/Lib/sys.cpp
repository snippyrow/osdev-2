#include "stdint.h"
#include "sys.h"

uint32_t call(uint32_t sysno, uint32_t a0, uint32_t a1, uint32_t a2)
{
    uint32_t ret;

    __asm__ volatile (
        "int $0x80"
        : "=a" (ret)
        : "a"  (sysno),                 // sysno → %eax
          "b"  (a2),                    // a0    → %ebx
          "c"  (a1),                    // a1    → %ecx
          "d"  (a0)                     // a2    → %edx
        : "memory", "cc"                // also mention condition codes if paranoid
    );

    return ret;
}