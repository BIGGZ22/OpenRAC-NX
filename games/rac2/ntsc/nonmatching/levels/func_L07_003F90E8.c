/* 0 of 2 places match - last line: 07_siberius: 192 bytes, expected 200
   TWO `nop`s from matching both places. Everything else is byte-exact, including the branch
   shape and every FP op.
   Inverse-lerp: `mid` is the small-data integer at $gp+0x2058 (0x2714 in level 16, so the
   symbol is per level); below a0 it returns lo, above a1 it returns hi, otherwise it returns
   lo + (hi - lo) * ((mid - a0) / (a1 - a0)) as four explicit CALLS to func_00283CE0, not casts
   - `(float)x` inline-folds to `cvt.s.w` and costs 92 bytes of body.
   THE FLAGS MATTER: the row only reaches 200 bytes with BOTH a `-g8` sibling profile and
   `__attribute__((sda))` on the declaration. With the file's default -G0 the load comes out
   `lui $v1,0x0 ; lw $v1,0x0($v1)`; at -G8 without `sda` it stays two words; `sda` alone at -G0
   also stays two words. Only `-O2 -G8 -ffunction-sections` plus `sda` gives the single
   `lw $v1,0x2058($gp)`. `-mno-split-addresses` on top changes nothing here (measured).
   The remaining gap, measured on six expression shapes (direct, product/numerator/
   denominator/fraction through a local, additive terms swapped): retail has two `nop`s between
   the last `sub.s` and the `div.s` that this cc1 does not emit, so every word from the
   `div.s` on is shifted by two - no expression shape moves them.
   What unblocks it: a lever that makes this cc1 emit its FP divide hazard padding. */

#include "include_asm.h"

extern int D_L07_001B1048 __attribute__((sda));

extern float func_00283CE0(int);

float func_L07_003F90E8(int a0, int a1, float lo, float hi)
{
    int mid = D_L07_001B1048;

    if (a0 >= mid)
        return lo;
    if (mid >= a1)
        return hi;
    return lo + (hi - lo) * ((func_00283CE0(mid) - func_00283CE0(a0)) /
                             (func_00283CE0(a1) - func_00283CE0(a0)));
}
