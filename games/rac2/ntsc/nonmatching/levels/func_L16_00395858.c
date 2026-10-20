/* func_L16_00395858 - 348 bytes, 2 place(s), level 16_snivelak, retail at
   0x00395858. Screened with tools/w10_screen.py: 2 call(s)
   [func_L00_002D2168@0039594C, func_L00_002D2168@003959A0]; data addresses
   materialised [0x190000, 0x3d4c0000, 0x3d4c0000]; float constants
   [0x40e00000, 0x41200000, 0x41200000, 0x41200000, 0x41200000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: lui $v1, 0x19 ; addiu $sp, $sp,
   -0x10 ; sd $ra, 0x0($sp) ; daddu $a3, $v1, $zero ; addiu $v1, $v1,
   -0x61E0 ; lw $v0, 0xC28($v1). Not matched in this wave: the body is
   straight-line code whose register allocation and the materialisation of
   its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
