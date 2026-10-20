/* 0 of 1 place match - last line: 01_oozla: 152 bytes, expected 144
   Six float arguments (four FP registers) and two pointers: scales *a1 by (1.0f - the fifth
   argument), squared, then blends it with *a0 - the third argument and the sixth argument,
   divides by the sixth argument squared, and leaves *a0 as the running total while the first
   argument goes to func_00282C48.
   Retail, word for word: `mul.s $f15,$f15,$f15` (r*r) ; `lwc1 $f1,0x0($a1)` ; the 1.0f - a
   fold, which is THREE operations in retail - `sub.s $f14,$f0,$f14` then
   `sub.s $f14,$f14,$f0` then `add.s $f14,$f14,$f0` - so the source carries a dead
   `(x - 1.0f) + 1.0f` that strict IEEE arithmetic forbids folding ; `mul.s $f1,$f1,$f14` and
   `swc1` ; then `lwc1 $f2,0x0($a0)`, `sub.s $f0,$f2,$f12`, `mul.s` (so *a0 is loaded once into
   $f2 and re-read by the sign test) ; `c.olt.s $f15,$f0` and the conditional is the branch-likely
   pair `bc1tl -> mul.s` with `mov.s $f0,$f15` in the slot, i.e. the then-arm jumps OVER the
   `mul.s $f0,$f0,$f13` ; `c.olt.s $f12,$f2` then `bc1fl` with `add.s $f0,$f1,$f0` in the slot
   and `sub.s $f0,$f1,$f0` as the fall-through.
   Measured once from a straightforward C: 152 bytes against 144. This cc1 loads *a0 before *a1
   (retail loads *a1 first), re-uses $f3 instead of $f2 for the first *a0 value, and - the size
   difference - needs $f20 and a 0x20 frame to keep the blend across the `div.s`, where retail
   needs no callee-saved FP register at all; it also emits `bc1f` + an explicit `b` where retail
   inverts to `bc1tl`.
   What unblocks it: a spelling that stops $f20 being allocated (the whole body then fits the
   0x10 frame) and gets the branch inverted. */

#include "include_asm.h"

extern float func_00282C48(float);

INCLUDE_ASM("asm/levels", func_L01_003CE7D8);
