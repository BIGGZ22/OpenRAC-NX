/* func_L00_00410430, 216 bytes, 27 places.  ABANDON - three words over on the big call.
   Read in full: with a nonzero third argument it stamps p[0x34] = arg, p[0x2C] = arg-1,
   func_002E5CF8(5, 0, a0), p[0x28] = arg-1 and func_L00_00410508(a0, a1); with a zero third
   argument it calls func_002E5CF8(5, 0, a0) then
   func_L00_003AB9C0(a0, a1, p[0], 0x001B8F20, *(float *)(p+8), 2.0f, 5.0f, 0.6f) - four
   integer arguments in a0-a3 and four floats in f12-f15 - and when that returns nonzero
   does func_L00_002D3F30(-1, 1), stores 1 in 0x001B2D58 and r[0xB8] = a0.
   Both shapes measured are 228 bytes against 216 (three words over), first differing word
   +0x14: the constant and address materialisation around the eight-argument call is what is
   left. Note the two stores in the third branch are `*(int *)(p + 0x2C) = a2 - 1` and
   `*(int *)(p + 0x28) = a2 - 1`, both recomputed rather than shared.
   What would unblock it: the emitted order of the a3 address and the f13-f15 constants. */

#include "include_asm.h"
