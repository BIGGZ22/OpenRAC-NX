/* 0 of 1 places match - catalogue entry is 8 bytes early (boundary defect).
   Retail bytes at the entry (level 7, 0x003791E8):
     003791E8  addiu $sp,$sp,0x10       <- NOT a prologue (deallocates the frame)
     003791EC  nop
     003791F0  addiu $sp,$sp,-0x10       <- the real prologue starts here
     003791F4  sd    $ra,0x0($sp)
     003791F8  jal   func_306800
     003791FC  nop
     00379200  ld    $ra,0x0($sp)
     00379204  jr    $ra
     00379208  addiu $sp,$sp,0x10
   The catalogue entry [0x003791E8, +0x24) therefore opens on an orphan `addiu
   $sp,$sp,+8` / nop pair -- the same epilogue fragment the real body ends with at
   0x00379208. The real body is 28 bytes at 0x003791F0 (the previous function,
   func_L07_00379180, ends at 0x003791E4 with its own epilogue delay slot).
   Needs the catalogue entry moved to 0x003791F0 with size 0x1C before any C can
   be placed; the body itself is a one-call wrapper (plain C, not yet built).

   Measured: a census of the whole level catalogue finds 6043 placements of 106337
   that open with `addiu $sp,$sp,+N ; nop`, and not one of them has a C definition
   anywhere in src/: none of these rows is placeable. The same shape is already
   recorded for func_L01_002E1670, func_L07_00323350 and func_L00_00341798.

   The wrapper below compiles to the retail's 28 bytes at 0x003791F0 instruction for
   instruction, except that this cc1 puts the `ld $ra,0x0($sp)` in the jal delay slot
   where retail leaves a nop there. */

#include "include_asm.h"

extern void func_00306800(void);

void func_L07_003791E8(void)
{
    func_00306800();
}
