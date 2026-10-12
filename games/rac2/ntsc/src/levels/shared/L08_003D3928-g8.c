#include "common.h"

/* The one body of L08_003D3928.c whose retail bytes reach 8_tabora's data through $gp:
   a store in ordinary flow, which only the small-data profile (-G8, from this file's
   name) reaches, as a one-instruction form.  The `sda` declaration is what makes cc1
   emit the bare symbol operand and the `.extern` the assembler needs. */

extern s32 D_L08_001B1FA8 __attribute__((sda));

extern s32 func_L08_003289D0(s32);

/* Totals three table lookups into one $-gp word and returns it. */
s32 func_L08_00435938(void)
{
    s32 first = func_L08_003289D0(0x96A);
    s32 second = func_L08_003289D0(0x1002);
    s32 sum = first + second + func_L08_003289D0(0x8C4);

    D_L08_001B1FA8 = sum;
    return sum;
}
