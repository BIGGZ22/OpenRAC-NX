/* func_L02_00401298, 4 bytes, 1 placement (level 2, 02_maktar_nebula).
   Not a C row: catalogue defect.  The four-byte extent at 0x00401298 is a
   single `addiu $sp,$sp,0x70` - a stack adjustment with no prologue, no
   `jr $ra` and no code around it; it is the tail of the neighbour's frame, the
   same shape as func_L00_0031AFF0 / func_L00_00358670 (see the README entry
   for func_L06_0035B768).  No C can be placed there: one C function cannot be
   one `addiu $sp,$sp,N`.  A `tools/levels.py` split (the row starts eight
   bytes late) is what unblocks it. */

#include "include_asm.h"
