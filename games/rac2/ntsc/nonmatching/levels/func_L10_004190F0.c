/* func_L10_004190F0, 152 bytes, 2 places.  ABANDON - store scheduling around a call.
   Read in full: `r = func_37BA88(a0)`, `p = *(char **)(a0 + 0x68)`, `*(int *)(r + 0x148) =
   10`, `func_37CCB0(a0, 1)`, then the state stores (p+0x180 = 10.0f, a0[0x20] = 1,
   a0[0x30] = 0xFF, a0[0x32] = 0x1FF, a0[0x34] |= 0x100, a0[0x2C] = 0.08f, p+0x268 = 0,
   p+0x26C = -1) and `func_37EFB8(a0)`.
   The candidate is the right size with the right instruction set and frame; two scheduled
   differences remain, both around the second call: retail writes `sw $v1,0x148($v0)` BEFORE
   the `a1 = 1` set-up and the jal, and puts the second call's delay slot on
   `addiu $a1,$zero,0x1`, while this cc1 hoists the set-up and sinks the store into the jal
   slot (+0x24 is the first differing word). Swapping the two state-store statements was
   measured and did NOT move the 0x34/0x2C pair (retail stores the 0x34 halfword first).
   What would unblock it: a source shape that keeps the store ahead of the call's argument
   set-up. */

#include "include_asm.h"
