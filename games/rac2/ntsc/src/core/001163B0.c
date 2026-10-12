#include "common.h"
#include "structs.h"
#include <stdarg.h>

extern void *r1_D_0012F86C NOT_SDA;

/* rac1-decomp src/core/00116070.c: func_001160D8 */
/* newlib's rand(), as newlib writes it: one expression that reads
   _REENT (r1_D_0012F86C) twice. +0x58 is _REENT->_next, which srand
   (r1_func_001160C8 above) sets. */
int func_001163B0(void) {
    return (*(unsigned int *)((char *)r1_D_0012F86C + 0x58) =
            *(unsigned int *)((char *)r1_D_0012F86C + 0x58) * 1103515245 + 12345) & 0x7FFFFFFF;
}

INCLUDE_ASM("asm/core", func_001163E0);

INCLUDE_ASM("asm/core", func_00116460);

INCLUDE_ASM("asm/core", func_001166C8);

INCLUDE_ASM("asm/core", func_00116E10);

INCLUDE_ASM("asm/core", func_00117108);

INCLUDE_ASM("asm/core", func_00117278);

INCLUDE_ASM("asm/core", func_001175F0);

INCLUDE_ASM("asm/core", func_00117650);

INCLUDE_ASM("asm/core", func_00117848);

INCLUDE_ASM("asm/core", func_00117CA0);

INCLUDE_ASM("asm/core", func_001182B8);

INCLUDE_ASM("asm/core", func_00118460);

INCLUDE_ASM("asm/core", func_001184D0);

INCLUDE_ASM("asm/core", func_00118548);

INCLUDE_ASM("asm/core", func_001185E8);

INCLUDE_ASM("asm/core", func_00118BC0);

INCLUDE_ASM("asm/core", func_00118CC8);

INCLUDE_ASM("asm/core", func_00118D98);

INCLUDE_ASM("asm/core", func_00119AC0);

INCLUDE_ASM("asm/core", func_00119BC8);

INCLUDE_ASM("asm/core", func_00119BF8);

INCLUDE_ASM("asm/core", func_0011A7F8);
