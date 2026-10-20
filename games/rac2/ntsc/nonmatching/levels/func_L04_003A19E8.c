/* 0 of 5 places match - last line: 04_barlow: instruction at +0xc differs.
   The loop is exact - the guard on the count, the per-entry call, the 0x10 and
   0x1190 pointer steps, the `nop` before the body - but the two pointers take
   each other's saved register: this cc1 gives the first parameter $s2 and the
   second $s1, retail the other way round, and it copies the count into $s0
   before the guard instead of testing the parameter first.  Two shapes
   measured (if + do-while and while); the declaration order does not move it. */

#include "include_asm.h"

typedef unsigned char u8;
typedef int s32;

extern void func_L04_0039FC28(u8 *, u8 *);

/* steps the second pointer by 0x10 and the first by 0x1190 over COUNT entries */
void func_L04_003A19E8(u8 *src, u8 *dst, s32 count) {
    while (count > 0) {
        func_L04_0039FC28(src, dst);
        dst += 0x10;
        count--;
        src += 0x1190;
    }
}
