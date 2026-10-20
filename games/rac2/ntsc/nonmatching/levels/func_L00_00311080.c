/* 0 of 27 places match - last line: 00_aranos_tutorial: 92 bytes, expected 100
   Candidate is 92 bytes against retail's 100 and holds the same instruction set: the same
   int-to-float conversion and division, both source bytes, the two byte arguments, the
   float at +0x2C, the cleared word at +0x24, 1.0f at +0x28 and the conditional tail call.
   Two differences, both scheduling: retail fills the load-to-store window with two `nop`s
   between `lbu $v0,0x43($a0)` and `div.s` and keeps the +0x22 store first, this cc1 puts
   `div.s` straight after the load and hoists the +0x23 store over the +0x22 one.
   Five source shapes tried (object bytes in locals first, the +0x20 pair stored first,
   the division moved after the stores, cast byte arguments, the natural order); all five
   give the identical 92-byte object, so the order is the scheduler's, not the source's. */

#include "include_asm.h"

extern void func_L00_00310D30(void);

void func_L00_00311080(unsigned char *a0, unsigned char *a1, int a2, int a3, int t0, int t1)
{
    float f = 1.0f / (float)t0;

    a1[0x22] = a0[0x43];
    a1[0x23] = a2;
    a1[0x20] = a0[0x41];
    a1[0x21] = a3;
    *(float *)(a1 + 0x2C) = f;
    *(int *)(a1 + 0x24) = 0;
    *(float *)(a1 + 0x28) = 1.0f;
    if (t1 != 0)
        func_L00_00310D30();
}
