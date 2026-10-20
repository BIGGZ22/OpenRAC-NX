/* Not matched (worker 06, level-20 wave of 2026-10-20).

   This body belongs in the `-G8` unit: it reads a global through
   `$gp` (`lw $a0,-0x2D0($gp)`), which only comes out of a file built
   with `-G8`, so it cannot live in the -G0 unit
   (src/levels/20_yeedil/L20_0039C448.c).  It also has the 128-bit
   wall: the retail's copy is `lq $v0,0x0($s0)` with `addiu $s0,$s0,0x10`
   (the object's +0x10) and `sq $v0,0x0($a2)`, where this compiler
   folds the offsets into the instructions (see the note of
   func_L20_0039C448 for the mechanism).

   The body is 1028 bytes with a frame of 0x600 and ten callee-saved
   registers. */
