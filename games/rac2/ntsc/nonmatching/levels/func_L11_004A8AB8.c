/* func_L11_004A8AB8, 4 bytes, 1 placement (level 11, 11_joba).
   Not a C row: catalogue defect.  The four-byte extent at 0x004A8AB8 is a
   single `addiu $sp,$sp,0x10` - a stack adjustment with no prologue, no
   `jr $ra` and no code around it; the tail of the neighbour's frame, the
   same shape as func_L11_004A0CE0 and func_L11_004A6B50.  No C can be
   placed there: one C function cannot be one `addiu $sp,$sp,N`.  A
   `tools/levels.py` split is what unblocks it. */

#include "include_asm.h"
