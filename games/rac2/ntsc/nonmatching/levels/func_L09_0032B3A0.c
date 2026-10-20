/* func_L09_0032B3A0 (4 bytes, 1 place: 09_dobbo 0x0032b3a0) -- ABANDON, catalogue defect:
   a fragment of a neighbouring body, not a function.
   The row holds one word, `addiu $sp,$sp,0x80` (0x27BD0080): the stack restore of the
   function that ends at 0x0032b3a0, i.e. the delay slot of its `jr $ra`. The next real
   body is func_L01_00324410 (a shared body, 316 bytes, 09_dobbo 0x0032b3a8 and ten other
   levels) and it starts with its own prologue, so the four bytes before it belong to the
   previous function, not to a body of their own. There is no `jr $ra` in the row, no
   register traffic and no prologue: no C spelling reaches this extent. The entry should
   be folded into a tools/levels.py split, not attempted.
   No C is deposited. */

#include "include_asm.h"
