/* func_L01_002B46A0, 100 bytes, 5 places.  ABANDON - one address form away.
   Read in full: `func_2B4380(0, 1.0f)` then, when the float at D_L01_00189E20+0x1B0 is
   positive, `*(float *)(D + 0x1B0) = D_001A7DC0 * (*(float *)(D + 0x24B8) * (1/60))`.
   The candidate is 100 bytes with every instruction present, but this cc1 materialises the
   FIELD address (base+0x1B0) and reaches the second field at 0x2308 off it, or makes a
   redundant copy of the base into $v1 (`lui $v0; addiu $v1,$v0,lo`), where retail keeps the
   BASE in $v0 and uses both 0x1B0 and 0x24B8 off it. Four source shapes measured (base
   pointer declared before/after the call, one expression, two temporaries). The first
   differing word is +0x24, in the borrow chain to 0x1B0.
   What would unblock it: an address-selection or register-allocation lever for a function
   whose base is live across a call, not the frame (the frame and the f12 argument are right).

   Candidate kept in nonmatching only; the C is the wave-6 draft in this file's sibling. */

#include "include_asm.h"
