/* 0 of 27 places match - last line: 00_aranos_tutorial: 172 bytes, expected 176
   Reads the per-level block at 0x0022F7C0 (0x80 slot index, 0x84 live count, 0x148+index*16
   a resident limit): if the count is set, the caller's id is updated and the engine queue is
   fed one of two request words with the 0x80F0F0F0 colour.
   Shape notes worth keeping: the limit must be read BEFORE the call so the base dies in $a0
   and only the limit lives in $s0, and the branch must be written `if (r >= limit)` so the
   0x1128 arm is the fall-through (retail `bnez` jumps to the 0x1129 arm).  What stays: this
   cc1 hoists the inner call's `addiu $a0,$zero,0x1129` into the `bnez` delay slot and then
   keeps the base in $a1, retail leaves a `nop` and keeps it in $a0. */

#include "include_asm.h"

extern int func_3269A8();
extern int func_2F4668(int);
extern void func_2E9B40(int, int, unsigned int, int, int);

extern char ref_L00_00378DE0_00[];
extern int ref_L00_00378DE0_01 __attribute__((sda));
extern int ref_L00_00378DE0_02;

void func_L00_00378DE0(void)
{
    char *p = ref_L00_00378DE0_00;
    int limit;
    int r;

    if (*(int *)(p + 0x84) == 0)
        return;
    limit = *(int *)(p + (*(int *)(p + 0x80) << 4) + 0x148);
    r = func_3269A8(p);
    if (r >= limit)
        func_2E9B40(ref_L00_00378DE0_01, ref_L00_00378DE0_02, 0x80F0F0F0,
                    func_2F4668(0x1128), -1);
    else
        func_2E9B40(ref_L00_00378DE0_01, ref_L00_00378DE0_02, 0x80F0F0F0,
                    func_2F4668(0x1129), -1);
}
