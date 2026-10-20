/* func_L13_0041BAE0 - 708 bytes, 2 place(s), level 13_boldan, retail at
   0x0041bae0. Screened with tools/w10_screen.py: 5 call(s)
   [func_00283658@0041BB24, func_0027C878@0041BB40, func_00283180@0041BCEC,
   func_00283198@0041BD00, func_00280B90@0041BD58]; data addresses
   materialised [0x1b0000, 0x3f930000, 0x3dcc0000, 0x3f260000, 0x3dcc0000,
   0x190000, 0x190000, 0x3e490000, 0x40490000]; float constants [0x3f800000,
   0x3f800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7A020000@0x0054,
   LQ/SQ 7C620000@0x0058, COP1 4680A520@0x0200]. Head: addiu $sp, $sp,
   -0x140 ; sd $s0, 0xD0($sp) ; sd $s1, 0xD8($sp) ; daddu $s0, $a0, $zero ;
   sd $s2, 0xE0($sp) ; addiu $s1, $sp, 0x90. Not matched in this wave: the
   128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
