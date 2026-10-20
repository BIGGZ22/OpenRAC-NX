/* Near miss from src/levels/20_yeedil/L20_0039C448-g8.c (the -G8 unit).

   No C was written for this body in this wave; what is recorded here is
   the structure that was mapped out of the retail and the walls it runs
   into, so the next attempt starts from the structure and not from the
   disassembly.

   func_L20_0039E368 is 648 bytes.  Its shape:

     - the prologue interleaves the body with the register saves (the
       retail's `daddu $s2,$a0` sits between `sd $s1` and `sd $ra`, the
       `lui $v1,%hi(D_001A8F00)` before the saves), which is the
       prologue-interleaving wall;
     - it calls f_0039E5F0 (the function that sits in the same unit)
       first and then dispatches on the byte at +0x20 of its argument
       through a JUMP TABLE, `jtbl_002B4CA0` (level address 0x002B4CA0,
       five entries, the `sltiu $v1,$a0,0x5` guard), so the C needs a
       `switch` whose table lands in .rodata at that address.  A C
       switch puts its table in the unit's own .rodata, which the build
       cannot place at the retail's address - the wall that keeps
       func_00118CC8 and func_00123C28 in nonmatching/ on the boot
       side.  That has to be solved before the body can land;
     - the switch's arms call f_0033E788, f_0032F2D0, f_00326528,
       f_00326488 and f_00302548, and the last one is called with the
       constant 0x3C888889 (1/60 as a float, one ulp above the natural
       0.016666666f - the known float-constant wall);
     - it reaches D_001A8F00, D_1BDCF0, D_1B2AC8 and D_1B2BE8 through
       $at/$lo and D_L20_001B2BE8's neighbours through $gp.

   The branch-likely form that the retail uses for its early exits
   (see the note on func_L20_0039D680) is the second wall after the
   jump table: the check block has to end in an unconditional jump to
   the body for this cc1 to invert the last condition.
*/
#include "include_asm.h"

/* ------------------------------------------------------------------
   Wave 21, worker 08.  THE JUMP TABLE ESCAPE WORKS.

   The wall that kept this body out of src/ is gone: the level's own
   jump table can be named and used with a computed goto, and the
   compiler emits exactly the retail's dispatch:

     extern void *D_L20_002B4CA0[5];
     ...
     goto *D_L20_002B4CA0[sel - 1];

   compiles to

     lui   $v0,%hi(D_L20_002B4CA0)
     sll   $a0,$a0,2
     addiu $v0,$v0,%lo(D_L20_002B4CA0)
     addu  $a0,$a0,$v0
     lw    $v1,0($a0)
     jr    $v1
     nop

   which is the retail's 0x39E3C4..0x39E3DC instruction for
   instruction (the retail keeps the index in $a0 and jumps through
   $a0; the attempt above keeps it in $v1).  The object gets
   R_MIPS_HI16/LO16 against the symbol and the build resolves the
   value from the retail's `lui %hi`/`addiu %lo` pair, so
   `D_L20_002B4CA0` is placed at 0x002B4CA0 and the table's CONTENTS
   (the retail's case addresses) come from the level's own data
   section.  The declaration has to have a size (5 here): an
   `extern void *D_L20_002B4CA0[];` never takes the absolute form in
   a -G8 file.

   The table, as the level's data has it:

     jtbl_002B4CA0 (0x002B4CA0), five entries:
       [0] 0x0039E5D4   (the epilogue: `ld $s0,0x60($sp)`)
       [1] 0x0039E3E0
       [2] 0x0039E430
       [3] 0x0039E51C
       [4] 0x0039E56C

   The dispatch's index is `sel - 1` where sel is the byte at 0x20 of
   the argument, so sel = 1 goes to the epilogue, sel = 2 to 0x39E3E0,
   sel = 3 to 0x39E430, sel = 4 to 0x39E51C and sel = 5 to 0x39E56C;
   the range check is `sltiu $v1,$a0,0x5` and the default is
   0x39E5CC (`jal f_0039E770`).

   So the source's shape is

     if (D_001A8F00 == 2) {
         if (D_1BDCF0 >= 5) { func_003253A8(arg0); goto out; }
         sel = *(u8 *)(arg0 + 0x20);
     } else {
         sel = *(u8 *)(arg0 + 0x20);
     }
     if ((u32)(sel - 1) >= 5) { goto default; }
     goto *D_L20_002B4CA0[sel - 1];

   with the five case bodies laid out in the source order case 2, case
   3, case 4, case 5, default, case 1 (the epilogue), because the
   retail's layout is 0x39E3E0, 0x39E430, 0x39E51C, 0x39E56C,
   0x39E5BC, 0x39E5CC, 0x39E5D4.

   What is left to do for this body: the five case bodies have to be
   written so that the compiled layout lands on the retail's
   addresses, and the branch-likely forms of the retail's early exits
   (`beql $v0,$zero` at 0x39E3E8, `bnel` at 0x39E470) have to be
   reproduced (the check block has to end in an unconditional jump to
   the body, see the note on func_L20_0039D680).

   The attempt below is unchanged: no C was written for this body in
   this wave, the structure above is what the next attempt should
   start from.
*/
