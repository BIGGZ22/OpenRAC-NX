#include "include_asm.h"

/* Select one of three measured pointer slots for the requested kind. */
extern void *D_0018C0B0;

extern void *D_0018B134;

extern void *D_0018B040;

/* Return the complete supplied unsigned-long 64-bit value unchanged. */
/* Target: FUN_003427B0 — identity function returning its first argument */
/* Disassembly: jr ra; move v0,a0 (MIPS delay slot) */
/* Compiled with: -O2 -G0 -ffunction-sections */

unsigned long func_L00_002A4EF0(unsigned long param_1)
{
  return param_1;
}

INCLUDE_ASM("asm/levels", func_L00_002A4EF8);

/* select primary pointer forkind0/2/3/4/5, secondary for1,tertiary for6; otherwiseNULL */
void *func_L00_002A4FE8(int kind)
{
    if (kind == 0 || kind == 2 || kind == 3 || kind == 4 || kind == 5)
        return D_0018C0B0;
    if (kind == 1)
        return D_0018B134;
    if (kind == 6)
        return D_0018B040;
    return 0;
}

INCLUDE_ASM("asm/levels", func_L00_002A5048);
