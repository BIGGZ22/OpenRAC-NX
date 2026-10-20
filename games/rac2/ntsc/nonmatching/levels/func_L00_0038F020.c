/* func_L00_0038F020 - 340 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038f020. Screened with tools/w10_screen.py: 6 call(s)
   [func_00283CE0@0038F088, func_00283CE0@0038F0A8, func_002AA140@0038F0C4,
   func_00277F48@0038F0DC, func_002A9850@0038F124, func_0029F338@0038F144];
   data addresses materialised [0x140000, 0x260000, 0x3d880000, 0x1c0000,
   0x830000]; float constants [0x3f800000, 0x3f000000, 0x40400000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 7A630010@0x00cc, LQ/SQ 7FA30000@0x00dc].
   Head: addiu $sp, $sp, -0x60 ; sd $s0, 0x10($sp) ; sd $s1, 0x18($sp) ; lui
   $s0, 0x14 ; sd $s2, 0x20($sp) ; addiu $s0, $s0, -0x6A98. Not matched in
   this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it
   only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
