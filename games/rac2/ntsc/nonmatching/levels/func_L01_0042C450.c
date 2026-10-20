/* func_L01_0042C450, 12 bytes, 1 placement (L01 0x0042c450).  ABANDON - catalogue defect,
   fragment of a neighbouring body, not a function.
   The previous row func_L00_00429020 ends at 0x0042c44c on `sb $a1,0x38($v0)` / `jr $ra` /
   `sb $zero,0x39($v0)`. The extent then reads `sb $zero,0x38($v0)` / `nop` /
   `sltiu $v0,$v0,0x1` and a `nop` fills 0x0042c45c; the next real body func_L00_004290C0
   starts at 0x0042c460 (12 places, L01 0x0042c460).
   Two of the three words are a recycled `sb`/`sltiu` pair of the same object shape as the
   neighbour's tail and `$v0` is live-in, never defined; there is no prologue, no `jr $ra` and
   no store that could follow the compare. No C spelling reaches this extent - the entry should
   be folded into a tools/levels.py split, not attempted.
   No C is deposited. */

#include "include_asm.h"
