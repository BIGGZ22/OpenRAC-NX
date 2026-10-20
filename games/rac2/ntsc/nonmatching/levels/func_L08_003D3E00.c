/* 0 of 1 place match - not attempted in C: the row carries a 128-bit transfer word.
   At 0x003D3E5C, in the middle of the mode dispatch, the retail word is 0x7FA00000 -
   opcode 0x1F (SQ), rs = $sp, rt = $zero, offset 0 - i.e. `sq $zero, 0x0($sp)`, a 16-byte
   zero written to a stack local, which rabbitizer prints as INVALID.
   It is the MMI/128-bit class the brief sends straight to a near miss, and the sixth wave's
   note applies: the TI128 `mode(TI)` typedef does reach lq/sq, but this cc1 folds a constant
   destination offset into the store where retail materialises the pointer first - a wall
   reconfirmed three times by the seventh wave, so no spelling was spent here.
   What the rest of the row shows: switch on the byte at object+0x20, case 0 sets 0x20(p)=1,
   case 1 tests object[0xBC] and either promotes to mode 2 or falls through, mode 2 runs
   func_002B0A68(object, 3.0f, 6.0f, &stack, 0x7F, 0xFF, 0.15f, 3.0f, 6.0f) and then
   func_0029F338(object).
   What unblocks it: the same TI128 destination lever as func_L00_002D5098. */

#include "include_asm.h"

INCLUDE_ASM("asm/levels", func_L08_003D3E00);
