/* func_L09_00326E88 (4 bytes, 1 place: 09_dobbo 0x00326e88) -- ABANDON, catalogue defect:
   a fragment of a neighbouring body, not a function.
   The row holds one word, `addiu $sp,$sp,0x150` (0x27BD0150): the stack restore of the
   function that ends at 0x00326e88, i.e. the delay slot of its `jr $ra`. The next real
   body is func_L09_00326E90 (a shared body, 108 bytes, 09_dobbo 0x00326e90 and
   20_yeedil 0x0033f820) and it starts with its own prologue
   (`lui $v1,0x19; addiu $sp,$sp,-0x10; ...`), so the four bytes before it belong to the
   previous function, not to a body of their own. There is no `jr $ra` in the row, no
   register traffic and no prologue: no C spelling reaches this extent. The entry should
   be folded into a tools/levels.py split, not attempted.
   No C is deposited. */

#include "include_asm.h"
