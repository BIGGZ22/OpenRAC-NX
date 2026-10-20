/* func_L00_002D3248, 68 bytes, 27 placements.  ABANDON - store-order wall.
   Writes a pair of clocks and a small record into the resident block: two floats (+0x1430,
   +0x1434), the 16-bit first argument stored wide at +0x1438, the second byte at +0x143C, and
   the last byte pair spliced into the existing word as (old & ~arg3) | arg2. The convention is
   four integer arguments (a0-a3: short, uchar, uchar, uchar) then two floats ($f12, $f13).
   Candidate below is 68 bytes and every instruction matches; only the store order differs.
   Retail emits +0x1434, +0x1438, +0x143C, +0x143E and leaves +0x1430 for the `jr` delay slot;
   this cc1 emits +0x1430 first and reaches +0x143E in the delay slot. Two source orders were
   measured (both clocks first, and the retail order with the +0x1430 store written last) and
   both come out the same.
   Unblocking: a store order that keeps the +0x1430 store last, or a declaration shape that
   keeps the FP store from being scheduled first. */

#include "include_asm.h"

extern char D_00189E20_2[];

void func_L00_002D3248(short arg0, unsigned char arg1, unsigned char arg2, unsigned char arg3,
                       float arg4, float arg5)
{
    char *base = D_00189E20_2;

    *(float *)(base + 0x1434) = arg5;
    *(int *)(base + 0x1438) = arg0;
    *(short *)(base + 0x143C) = arg1;
    *(short *)(base + 0x143E) = (*(unsigned short *)(base + 0x143E) & ~arg3) | arg2;
    *(float *)(base + 0x1430) = arg4;
}
