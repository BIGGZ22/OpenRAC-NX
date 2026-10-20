/* 0 of 27 places match - last line: 00_aranos_tutorial: instruction at +0x0 differs.
   Four 128-bit block copies (lq/sq) plus a $gp store of the advanced source
   pointer.  The $gp store needs a -G8 unit: this body was measured in
   src/levels/shared/L00_002FD480.c with `-O2 -G8 -ffunction-sections` (a file of
   its own, since the canonical L00_002FC7A0.c unit is -G0 and turned the store
   into lui+sw, four bytes longer).  With -G8 the store is `sw $a0,%lo($gp)`
   like retail.  What remains is the register assignment: retail keeps the
   destination address in $v1 and the loaded block in $v0 and reuses $v1 for all
   four blocks, this cc1 loads into $v1 and materialises each destination into a
   different register.  The four destinations are materialised absolutely in the
   retail bytes, so they carry nosda. */

#include "include_asm.h"

typedef int TI128 __attribute__((mode(TI)));

extern TI128 D_L00_001B1D20 __attribute__((nosda));
extern TI128 D_L00_001B1D30 __attribute__((nosda));
extern TI128 D_L00_001B1D40 __attribute__((nosda));
extern TI128 D_L00_001B1D50 __attribute__((nosda));
extern TI128 *D_L00_001B1D60 __attribute__((sda));

/* copies four sixteen-byte blocks into the level's upload buffer and parks the
   advanced source pointer in the cell that follows them */
void func_L00_002FD480(TI128 *src)
{
    TI128 *dst;

    dst = &D_L00_001B1D20;
    *dst = *src++;
    dst = &D_L00_001B1D30;
    *dst = *src++;
    dst = &D_L00_001B1D40;
    *dst = *src++;
    dst = &D_L00_001B1D50;
    *dst = *src++;
    D_L00_001B1D60 = src;
}
