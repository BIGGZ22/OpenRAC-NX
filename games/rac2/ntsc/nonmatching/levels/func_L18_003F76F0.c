/* func_L18_003F76F0 - 296 bytes, 3 place(s), level 18_damosel, retail at
   0x003f76f0. Screened with tools/w10_screen.py: 2 call(s)
   [func_L18_0032B768@003F7778, func_002A7750@003F7788]; data addresses
   materialised [0x3c230000, 0x3efa0000]; float constants [0x40000000,
   0x40a00000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [COP1 46800020@0x0054,
   LQ/SQ 7A420000@0x00bc, LQ/SQ 7CA20000@0x00c0, LQ/SQ 7A620000@0x00c8,
   LQ/SQ 7C620000@0x00cc]. Head: addiu $sp, $sp, -0x30 ; sd $s1, 0x8($sp) ;
   sd $s2, 0x10($sp) ; daddu $s1, $a0, $zero ; sd $s3, 0x18($sp) ; daddu
   $s2, $a1, $zero. Not matched in this wave: the 128-bit copy needs an
   `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local, and
   this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
