/* Near miss from src/levels/20_yeedil/L20_0039C448-g8.c (the -G8 unit).

   No C was written for this body in this wave; what is recorded here is
   the structure that was mapped out of the retail and the walls it runs
   into, so the next attempt starts from the structure and not from the
   disassembly.

   func_L20_00408D98 is 1760 bytes, the biggest body of this unit.  Its
   shape:

     - it dispatches on the byte at +0x20 of its argument through a JUMP
       TABLE, `jtbl_002B5340` (level address 0x002B5340, eight entries,
       the `sltiu $v0,$v1,0x8` guard), so the C needs a `switch` whose
       table lands in .rodata at that address.  A C switch puts its table
       in the unit's own .rodata, which the build cannot place at the
       retail's address - the wall that keeps func_00118CC8 and
       func_00123C28 in nonmatching/ on the boot side.  That has to be
       solved before the body can land, and it is the reason this body
       was not attempted further in this wave;
     - the prologue interleaves the body with the saves (the `lui
       $v1,%hi(D_1CA540)` and the `daddu $s4,$v1` sit between the
       `sd $s2` and the `sd $s0`), the prologue-interleaving wall;
     - the first arm writes the constant 0x42AA0000 (85.0f) through
       D_1B2BE8 with `lui $at` + `mtc1` + `swc1 %lo(D_1B2BE8)($at)` -
       an absolutely addressed float, so the declaration wants
       `nosda` (or a plain `extern float`, which the assembler expands
       into that form);
     - the arms use D_001A79F0 (the $gp base at 0x1AEFF0 - 0x7600) and
       D_0019B2E8 as table bases, with the same `sll ...,4` / `addu`
       shape as func_L20_003BFEB8.

   With 1760 bytes the body has to be worked in blocks against the
   --diff, one arm of the switch at a time; the jump table's placement
   is the first thing to solve.
*/
#include "include_asm.h"

/* ------------------------------------------------------------------
   Wave 21, worker 08.  THE JUMP TABLE ESCAPE WORKS HERE TOO.

   The wall that kept this body out of src/ is gone: the level's own
   jump table can be named and used with a computed goto, and the
   compiler emits exactly the retail's dispatch:

     extern void *D_L20_002B5340[8];
     ...
     goto *D_L20_002B5340[sel];

   compiles to

     lui   $v0,%hi(D_L20_002B5340)
     sll   $a0,$a0,2
     addiu $v0,$v0,%lo(D_L20_002B5340)
     addu  $a0,$a0,$v0
     lw    $v1,0($a0)
     jr    $v1
     nop

   The object gets R_MIPS_HI16/LO16 against the symbol, the build
   resolves the value from the retail's `lui %hi`/`addiu %lo` pair
   (0x002B5340), and the table's CONTENTS come from the level's own
   data section.  The declaration needs a size (8 here): an
   `extern void *D_L20_002B5340[];` never takes the absolute form in
   a -G8 file.

   The table, as the level's data has it:

     jtbl_002B5340 (0x002B5340), eight entries:
       [0] 0x00408E08
       [1] 0x00408EA8
       [2] 0x004090AC
       [3] 0x004090AC
       [4] 0x00409154
       [5] 0x004091E8
       [6] 0x0040925C
       [7] 0x004093BC

   The dispatch's guard is `sltiu $v0,$v1,0x8`, so the index is the
   byte itself (not shifted by one as in func_L20_0039E368).

   With 1760 bytes the body has to be worked arm by arm against the
   --diff; the jump table's placement is no longer the blocker, the
   layout of the eight arms is.

   The attempt below is unchanged: no C was written for this body in
   this wave.
*/
