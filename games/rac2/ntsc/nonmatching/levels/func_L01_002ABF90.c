/* func_L01_002ABF90, 12 bytes, 2 placements.  ABANDON - catalogue defect, linker fill
   between two real bodies, not a function.
   Place L01 0x002abf90: the previous row func_L00_002AA200 ends exactly here on a complete
   epilogue (`ld $ra,0x58($sp)` / `jr $ra` / `addiu $sp,$sp,0x70`), and the extent then reads
   `addiu $sp,$sp,0x40` / `nop` / `addiu $sp,$sp,0x70` - two stray stack restores with no
   prologue and no `jr $ra`. A `nop` fills the last four bytes and the next real body,
   func_L00_002AA938, starts at row+0x10 (L01 0x002abfa0, L07 0x002aa250). Place L07
   0x002aa240 is byte-identical, same neighbours, same +0x10 gap.
   No C can produce three words made only of `addiu $sp,$sp,+N` and a `nop`: the entry should
   be folded into the following row by a tools/levels.py split. Data for the maintainers -
   this is the `addiu $sp,$sp,+N; nop` family the brief lists.
   No C is deposited. */

#include "include_asm.h"
