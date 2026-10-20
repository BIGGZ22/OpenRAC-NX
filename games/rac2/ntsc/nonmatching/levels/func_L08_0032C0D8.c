/* func_L08_0032C0D8 - 312 bytes, 2 place(s), level 08_tabora, retail at
   0x0032c0d8. Screened with tools/w10_screen.py: 6 call(s)
   [func_00283180@0032C10C, func_00283198@0032C120, func_00283B98@0032C154,
   func_002AB1F0@0032C178, func_002A75E0@0032C1CC, func_002A7470@0032C1E4];
   data addresses materialised [0x40490000]; float constants [0x3f800000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: addiu $sp, $sp, -0x40 ; swc1 $f20,
   0x30($sp) ; sd $s0, 0x0($sp) ; mov.s $f20, $f12 ; sd $s1, 0x8($sp) ;
   daddu $s0, $a1, $zero. Not matched in this wave: the body is straight-
   line code whose register allocation and the materialisation of its
   constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
