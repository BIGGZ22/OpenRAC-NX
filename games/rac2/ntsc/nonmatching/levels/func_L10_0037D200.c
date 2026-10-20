/* func_L10_0037D200, 148 bytes, 2 places.  ABANDON - the loop test idiom.
   Read in full and the logic is measured: `p = D_001FB1E0[a0[0x21]]`, then a walk over the
   halfword list it points at, counting the entries whose record at
   `D_001B251C + ((h & 0x7FFF) << 8)` has bit 0x20 in its +0x34 halfword and a positive
   float at `*(float **)(record + 0x68)`; the list ends at the first halfword with bit 15
   set, and the count is the answer.
   The candidate is 140 bytes against 148: retail keeps the loaded halfword in a SECOND
   register ($a1 = the entry, $v1 = the index value) and tests it with `sll $v0,$a1,16;
   bgez`, re-materialising that shift in the annulled slot of the inner beql; this cc1
   instead reloads the halfword (`lh`) for the test and emits `bgezl` with the load in the
   slot, two words shorter. A named halfword variable tested as `(short)h >= 0` changed
   nothing (both spellings measured, 140 each).
   What would unblock it: the register pair / branch-likely form for the terminator test. */

#include "include_asm.h"
