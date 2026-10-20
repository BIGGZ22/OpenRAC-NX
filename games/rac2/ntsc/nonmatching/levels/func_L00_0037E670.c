/* 0 of 27 places match - last line: 00_aranos_tutorial: 48 bytes, expected 40
   A mode setter: mode == 0 parks 40.0f in the $gp slot, anything else parks 100.0f in
   the absolute word at 0x001B26B0; both paths then clear the $gp flag (retail keeps that
   store in the `jr $ra` delay slot).  Retail keeps the constant out of the branch
   (`lui $at,0x4220; mtc1 $at,$f0`) and uses the conditional-execute shape
   `beql $a0,$zero,join` with `swc1 $f0,0x36C0($gp)` in the annulled delay slot.
   Every source shape tried here (if/else in both arm orders, `!mode`, ternary, switch,
   goto, a named local constant, the flag store duplicated in both arms, flag store first)
   makes this cc1 emit the plain `bnez Lelse; nop; <then>; b join; <else>` form, 48 bytes,
   at -O2/-O3/-Os and under the sdk profile alike; the two shapes that did reach 40 bytes
   (flag duplicated in both arms, flag first) change the store order, so they are not this
   function.  `-mbranch-likely` is not a valid option for this cc1, and
   `-mastra-annul-dead-slots` (patch 0025, opt-in again since 0054) only annuls a slot whose
   destination is a REGISTER the branch tests, never a memory store.
   What would unblock it: a lever that makes this cc1 hoist the constant above the branch
   and annul a store-bearing delay slot (the retail `beql` conditional-execute form). */

#include "include_asm.h"

extern float ref_L00_0037E670_00;
extern int ref_L00_0037E670_01;
extern float D_L00_001B26B0;

void func_L00_0037E670(int mode)
{
    if (mode == 0)
        ref_L00_0037E670_00 = 40.0f;
    else
        D_L00_001B26B0 = 100.0f;
    ref_L00_0037E670_01 = 0;
}
