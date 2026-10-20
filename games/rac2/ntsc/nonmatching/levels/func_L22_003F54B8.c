/* 0 of 2 places match - last line: 22_dobbo_orbit: 200 bytes, expected 196
   Builds a three-float block from the record's +0x10 tail, steps the record, then sets its
   aim: the sign of the pi/2 constant (0xBFC90FDB / 0x3FC90FDB) follows the step's verdict,
   the stepped angle is folded in, and three resident words are stamped (the last with the
   2*pi/21600 per-frame turn rate).  Retail uses the annulled conditional store --
   `beql $v0,$zero,L; swc1 $f0,0xB0($s1)` with the NEGATIVE constant materialised above the
   branch -- which is the same conditional-execute wall as func_L00_0037E670; this cc1 emits
   the plain branch and both arms, one instruction over.  Two $gp floats stay ref_ names. */

#include "include_asm.h"

extern void func_2F3AB0(float *, void *, void *);
extern void func_329EB8(void *, void *, void *);
extern int func_31FC88(int);
extern float func_2F4138(float, float);
extern float func_2F4A88(float, float);
extern char ref_L22_003F54B8_00[];
extern f32 ref_L22_003F54B8_01 __attribute__((sda));
extern f32 ref_L22_003F54B8_02 __attribute__((sda));

void func_L22_003F54B8(u8 *a0)
{
    f32 block[4];
    u8 *s1;

    func_2F3AB0(block, ref_L22_003F54B8_00, a0 + 0x10);
    s1 = *(u8 **)(a0 + 0x68);
    func_329EB8(a0, block, block);
    if (func_31FC88(2) == 0)
        *(f32 *)(s1 + 0xB0) = -1.5707964f;
    else
        *(f32 *)(s1 + 0xB0) = 1.5707964f;
    *(f32 *)(s1 + 0xB0) = func_2F4A88(*(f32 *)(s1 + 0xB0),
                                      func_2F4138(block[0], block[1]));
    *(f32 *)(s1 + 0xC0) = ref_L22_003F54B8_01 * 0.000290888245f;
    *(f32 *)(s1 + 0xBC) = ref_L22_003F54B8_02;
    *(int *)(s1 + 0xC4) = 0;
}
