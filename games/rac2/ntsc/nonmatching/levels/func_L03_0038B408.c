/* func_L03_0038B408, 224 bytes, 2 places.  ABANDON - frame and loop rotation.
   Read in full: for the four ints at p+0x28 (p = *(char **)(a0+0x68)) it takes
   q = D_L03_001FE700[p[0x28+i*4]] and, when q is nonzero, runs
   func_L03_0038B2E8(buf, q, p[0x38]) then walks q+0x10 by 0x10 for p[0x38] entries calling
   func_00282CF0(v, v, buf) and func_00282CC0(v, v, p+0xE0).
   Two shapes measured, 204 and 212 bytes against 224: retail keeps the table base in $s5
   and saves s0-s5 in a 0x50 frame, both candidates reach the table through $s4 with a 0x40
   frame, and the inner walk is rotated differently (`v += 0x10` at the top of the body
   against the peeled form). The first differing word is +0x0C, the frame itself.
   What would unblock it: the register set for a table base that outlives two calls. */

#include "include_asm.h"
