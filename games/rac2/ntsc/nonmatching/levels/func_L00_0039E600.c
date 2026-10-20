/* func_L00_0039E600 - 420 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0039e600. Screened with tools/w10_screen.py: 6 call(s)
   [func_00282948@0039E648, func_0029F338@0039E6A8, func_00282C88@0039E6BC,
   func_00283098@0039E6DC, func_00282CC0@0039E6EC, func_00283CF0@0039E748];
   data addresses materialised [0x1b0000, 0x3c880000, 0x3f8c0000, 0x1b0000,
   0x1b0000, 0x3dcc0000]; float constants [0x3f800000, 0x41200000,
   0x42fe0000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 78620000@0x0100,
   LQ/SQ 7C820000@0x0104]. Head: lui $at, 0x1B ; lwc1 $f0, -0x5C($at) ;
   addiu $sp, $sp, -0x30 ; sd $s3, 0x18($sp) ; sd $s0, 0x0($sp) ; add.s $f0,
   $f0, $f0. Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
