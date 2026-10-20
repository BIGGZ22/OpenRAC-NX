/* func_L26_002EC840, 20 bytes, 1 placement (level 26 jamming array).  ABANDON - catalogue
   defect, the `addiu $sp,$sp,+N; nop` family the brief lists.  There is no body here:
   the extent 0x002EC840..0x002EC854 is `addiu $sp,$sp,0x80; nop; addiu $sp,$sp,0xC0; nop;
   addiu $sp,$sp,0xF0`, and the eight bytes that follow it (0x002EC850/0x002EC854) are the
   same bare restores plus a pad.  No prologue, no `jr $ra`, no register ever set: three
   stack restores and three pads.
   The neighbours pin the boundary and show nothing was cut off: the entry before ends at
   0x002EC83C with a complete `jr $ra; addiu $sp,$sp,0x30` (0x002EC838/0x002EC83C), and the
   entry after, func_L00_002EC608, opens at 0x002EC858 with a real prologue
   `addiu $sp,$sp,-0x130; lui $at,0xC000; mtc1 $at,$f1; swc1 $f20,0x120($sp)`.
   The same `addiu $sp,$sp,0xF0; nop` pair sits immediately before func_L00_002EC608 in
   level 0 too (0x002EC600/0x002EC604), where the catalogue hands it to the previous entry
   rather than making a function of it - so the level-26 cut is the odd one.  The extent
   needs re-deriving in tools/levels.py; no C can be placed on it.
   No C is deposited. */

#include "include_asm.h"
