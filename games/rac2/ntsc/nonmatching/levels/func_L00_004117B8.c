/* func_L00_004117B8, 236 bytes, 27 placements.  ABANDON - the address-CSE wall (260 bytes).
   Walks the 30 entries of the 0x00256620 block (stride 0x30, counter 0x1D down to 0) and, for
   every entry whose +0x18 pointer is set, folds the record into a 16-byte stack buffer:
   func_2AFFD8(rec + 0x10, buf, 1), func_282D30(buf, buf, *(f32 *)(rec + 0xC) * 0.0008300250046886504f
   = 0x3A599609), then buf[0] += *(f32 *)(rec + 0) * 0.0009764999849721789f (= 0x3A7FFBCE).
   Outside the guard the buffer takes the same scale from the two words at 0x00250004 and
   0x00250008, and the routine ends with func_L00_00411F48(a0).
   Retail materialises 0x00250000 once (`lui $v0,0x25`) and REUSES $v0 for the two out-of-guard
   loads (`lwc1 $f0,0x4($v0)`, `lwc1 $f0,0x8($v0)`) even across the calls, so the CSE'd %hi is
   still live where this cc1 parks the two addresses in callee-saved registers: the frame grows
   0x40 -> 0x50 with $s3/$s4 (260 bytes).  `nosda` on the three symbols gives the same 260, and
   spelling the two words as fields of the block gives the same 260. */

#include "include_asm.h"
#include "common.h"

extern char D_00250000[];

extern f32 D_00250004;

extern f32 D_00250008;

extern void func_002AFFD8(char *a0, void *a1, s32 a2);

extern void func_00282D30(void *a0, void *a1, f32 f12);

extern void func_L00_00411F48(char *a0);

void func_L00_004117B8(char *a0)
{
    char *s0 = D_00250000 + 0x6620;
    char *s2 = a0;
    char *p;
    f32 buf[4];
    s32 i;

    for (i = 0x1D; i >= 0; i--) {
        p = *(char **)(s0 + 0x18);
        if (p != 0) {
            func_002AFFD8(p + 0x10, buf, 1);
            p = *(char **)(s0 + 0x18);
            func_00282D30(buf, buf, *(f32 *)(p + 0xC) * 0.0008300250046886504f);
            p = *(char **)(s0 + 0x18);
            buf[0] = buf[0] + *(f32 *)(p + 0) * 0.0009764999849721789f;
        }
        buf[1] = buf[1] + D_00250004 * 0.0009764999849721789f;
        buf[2] = buf[2] + D_00250008 * 0.0009764999849721789f;
        s0 = s0 + 0x30;
    }
    func_L00_00411F48(s2);
}
