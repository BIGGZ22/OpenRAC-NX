/* from src/core/0011BEE0.c: kputchar, exact bytes only with a register pin */
/*
 * The 56-byte body comes out byte for byte with a local pinned to $v1 through an
 * empty asm statement, __asm__("" : "=r"(p) : "0"(0x1000F180)). AGENTS.md's plain-C
 * rule forbids inline assembly and register pins, so this body cannot land as C;
 * the marker is back in place. The same shape is what rac1-decomp records for its
 * kputchar. ASM_FUNC candidate on a maintainer's agreement.
 */
#include "common.h"
#include "structs.h"

/* kputchar (libkernl.a:kprintf.o): wait until the SIO status port
   (0x1000F130) has its busy bit (0x8000) clear, write the character to
   the SIO data port (0x1000F180) and return it.

   Retail's store is not volatile (it fills the return's delay slot, which
   a volatile access never does) yet its address stays in a register
   (lui/ori). A plain constant address is folded into the store by the
   optimizer instead, so the address goes through an empty asm that hides
   its value.

   Identity: config/datamine/ntsc-u-sdk-matches.tsv names 0x0011BEE0
   libkernl.a:kprintf.o kputchar; rac1-decomp src/core/00119D88.c:
   func_00119D88 is the same body. */
int func_0011BEE0(int c) {
    unsigned char *p;

    while (*(volatile unsigned int *)0x1000F130 & 0x8000)
        ;
    __asm__("" : "=r"(p) : "0"(0x1000F180));
    *p = c;
    return c;
}
