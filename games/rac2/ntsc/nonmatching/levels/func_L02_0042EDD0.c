/* func_L02_0042EDD0 - 476 bytes, 2 place(s), level 02_maktar_nebula, retail
   at 0x0042edd0. Screened with tools/w10_screen.py: 2 call(s)
   [func_002833F0@0042EEE0, func_00283738@0042EEF0]; data addresses
   materialised [0x1b0000, 0x1b0000, 0x1b0000, 0x3c000000, 0x1b0000,
   0x3c880000, 0x1b0000]; float constants [0x1b0000, 0x3f800000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 7FA00000@0x0100]. Head: addiu $sp, $sp,
   -0x80 ; sd $s0, 0x40($sp) ; sd $s1, 0x48($sp) ; sd $s2, 0x50($sp) ; sd
   $s3, 0x58($sp) ; sd $ra, 0x60($sp). Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
