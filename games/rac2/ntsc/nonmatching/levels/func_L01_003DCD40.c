/* best attempt - last check.py line:
   01_oozla: candidate is 144 bytes, expected 148
   The 128-bit copy does come out of plain C: a `mode(TI)` local (the trick
   src/game/002A7750.c uses) gives retail's `lq $v0,0($v1); sq $v0,0($s0)` pair,
   so no inline assembly is needed. What is left is the register rotation:
   retail copies a0 into $s1, the state pointer into $s2, a1 into $s3 and keeps
   &vec in $s0; this cc1 copies a0 into $s0, the state into $s1, a1 into $s2 and
   keeps &vec in $s3, and the prologue's save order follows (retail s1, s3, s0,
   ra, s2 - here s0, s2, s3, ra, s1). The stack layout, the frame, the three
   calls and the two 0x3E4 guards are otherwise exact. Twelve source shapes
   measured (the declaration order of the four locals, the pointer variables for
   the copy's source and destination, the copy written inline or through them):
   all 144, all with the same rotation. The two absolute globals need the -g8
   sibling and the `$gp` one needs -G8. */

#include "common.h"

typedef int TI128 __attribute__((mode(TI)));

extern void func_L01_003DC8B8(u8 *, void *, void *);
extern void func_L01_00318638(void *, void *, void *);
extern void func_L01_003DC9E0(void *, void *);
extern void func_L01_003DC9A8(void **, s32 *);

void func_L01_003DCD40(u8 *a0, void *a1) {
    u8 *o = (u8 *)*(s32 **)(a0 + 0x68);
    s32 list[0x10];
    TI128 vec;
    s32 count;
    TI128 *dst = &vec;
    TI128 *src = (TI128 *)(a0 + 0x10);

    if (*(u8 *)(o + 0x3E4) != 0) {
        func_L01_003DC8B8(a0, list, &count);
    }
    *dst = *src;
    func_L01_00318638(a0, a1, o + 0x1C0);
    func_L01_003DC9E0(a0, dst);
    if (*(u8 *)(o + 0x3E4) != 0) {
        func_L01_003DC9A8((void **)list, &count);
    }
}
