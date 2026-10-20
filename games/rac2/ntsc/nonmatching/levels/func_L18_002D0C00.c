/* func_L18_002D0C00 - 572 bytes, 3 place(s), level 18_damosel, retail at
   0x002d0c00. Screened with tools/w10_screen.py: 13 call(s)
   [func_L00_002B2BD0@002D0C1C, func_00282978@002D0C74,
   func_L00_002D3400@002D0C94, func_L00_002B3900@002D0CB0,
   func_00282E20@002D0CBC, func_L00_002B3900@002D0D2C,
   func_L18_002D7650@002D0D34, func_00283410@002D0DF0,
   func_00283388@002D0DFC, func_00283698@002D0E08,
   func_L18_002D0850@002D0E10, func_L00_002C2DE8@002D0E18,
   func_L18_002DEFF8@002D0E20]; data addresses materialised [0x1b0000,
   0x190000, 0x140000, 0x190000, 0x190000]; float constants [0x3e800000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 78820000@0x0038, LQ/SQ 7CA20000@0x003c,
   COP1 46800020@0x0114, COP1 46800860@0x0120, LQ/SQ 78C20000@0x01a8, LQ/SQ
   7C620000@0x01ac]. Head: addiu $sp, $sp, -0x20 ; lui $v0, 0x1B ; lw $v0,
   -0x450($v0) ; sd $s0, 0x0($sp) ; sd $s1, 0x8($sp) ; beqz $v0, . + 4 +
   (0x5 << 2). Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
