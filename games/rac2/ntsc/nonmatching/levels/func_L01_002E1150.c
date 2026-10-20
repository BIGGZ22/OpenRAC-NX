/* func_L01_002E1150, 12 bytes, 8 placements.  ABANDON - catalogue defect, linker fill
   before a shared body, not a function.
   Every place reads `addiu $sp,$sp,0xA00` / `nop` / `addiu $sp,$sp,0x100`, then a `nop` at
   row+0xC, then the prologue of func_L00_002EBE88 at row+0x10: no prologue, no `jr $ra`, no
   register traffic, and 0xA00 (2560 bytes) is larger than any frame in this unit.
   The same shape and the same +0x10 delta in all eight places:
     L01 0x002e1150 -> 0x002e1160   L05 0x00304788 -> 0x00304798
     L10 0x00306c88 -> 0x00306c98   L15 0x00304d88 -> 0x00304d98
     L22 0x002f1bd8 -> 0x002f1be8   L23 0x002f62b8 -> 0x002f62c8
     L24 0x002ea9a0 -> 0x002ea9b0   L25 0x002fed58 -> 0x002fed68
   The three words are the same two-restore fragment family as func_L01_002ABF90. The entry
   is spurious: it is not a second name for func_L00_002EBE88 either (that row is 27 places,
   no address is carried twice in the catalogue), it is the alignment padding the splitter
   turned into a row. Fix belongs in tools/levels.py.
   No C is deposited. */

#include "include_asm.h"
