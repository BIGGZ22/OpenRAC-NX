/* func_L13_00439A70 (112 bytes, 3 places) -- NEAR MISS, not integrated.
   state: everything from the record load to the epilogue matches exactly (`lw $v1,0x1C($v1)`
   in the bne delay slot, `beqz` with the nop, `addiu $v0,$zero,3` twice, `sh $v0,0x7E($a2)`,
   `jr $ra`); the residual is the resident-base materialisation order and register roles.

   Two measured forms, both 112 bytes:
   - plain `extern int D_L13_001B22C4 __attribute__((nosda));` (in a -g8 sibling, since the
     function reads $gp+0x3150): the emitted order is retail's (index, base, symbol load,
     shift, second short, addu, resident word, bne) but the symbol load comes out
     `lui $a1,$hi ; lw $v1,%lo($a1)` where retail has `lui $a1 ; lw $a1` (destination as
     base).
   - the same with the alias pair `extern int D_L13_001B22C4_v __asm__("D_L13_001B22C4");`
     used in the expression: the load becomes retail's `lui $v1 ; lw $v1` AND the bne delay
     slot matches, but the symbol load is then hoisted three slots above the index load.
   So the alias lever trades the load form for the schedule; a source shape that pins both
   is what is left. D_00189E20 is resident (0x189E20); D_L13_001B22C4 is level data
   (0x1B22C4) reached absolutely, D_L13_001B2140 (0x1B2140) through $gp (offset 0x3150). */
