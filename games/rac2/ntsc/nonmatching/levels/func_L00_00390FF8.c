/* func_L00_00390FF8 - 792 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00390ff8. Screened with tools/w10_screen.py: 21 call(s)
   [func_0029EFD0@00391034, func_002AD0B0@003910DC, func_00282DF0@003910EC,
   func_L00_00390E10@00391128, func_002A0F48@00391130,
   func_L00_004282F0@00391138, func_L00_002C42E0@00391148,
   func_00282A60@00391158, func_00282A60@00391168, func_0029F978@00391178,
   func_0029F978@00391188, func_L00_00395240@00391190,
   func_002AB650@003911B4, func_002E5CF8@003911C4, func_002761F8@003911F0,
   func_002B0CE0@00391228, func_002AFFD8@00391264, func_00282DD8@0039127C,
   func_00282F20@00391290, func_00282DD8@003912A0, func_002FED90@003912D0];
   data addresses materialised [0x190000, 0x1b0000, 0x1b0000, 0x190000,
   0x3ecc0000, 0x3c880000, 0x1c0000, 0x1b0000, 0x1b0000]; float constants
   [0x3f800000, 0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7A220000@0x00c8, LQ/SQ 7C620000@0x00cc, LQ/SQ 7A420010@0x01e4, LQ/SQ
   7FA20000@0x01f0, LQ/SQ 78620000@0x0214, LQ/SQ 7E620000@0x0218, LQ/SQ
   78820000@0x0258, LQ/SQ 7E420000@0x025c]. Head: addiu $sp, $sp, -0x70 ; sd
   $s2, 0x30($sp) ; sd $s1, 0x28($sp) ; daddu $s2, $a0, $zero ; swc1 $f21,
   0x68($sp) ; daddu $s1, $a1, $zero. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
