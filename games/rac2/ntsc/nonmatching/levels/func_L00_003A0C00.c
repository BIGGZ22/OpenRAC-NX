/* func_L00_003A0C00 - 412 bytes, 18 place(s), level 00_aranos_tutorial,
   retail at 0x003a0c00. Screened with tools/w10_screen.py: 4 call(s)
   [func_002A8AF0@003A0D04, func_002ACBC8@003A0D18,
   func_L00_002E6EF8@003A0D4C, func_L00_00391310@003A0D68]; data addresses
   materialised [0x190000, 0x200000, 0x3a0000]; float constants [0x1b0000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: addiu $sp, $sp, -0x50 ; addiu $v0,
   $zero, 0xFF ; sd $s5, 0x28($sp) ; addiu $v1, $zero, 0x1C ; sd $s3,
   0x18($sp) ; daddu $s5, $a0, $zero. Not matched in this wave: the body is
   straight-line code whose register allocation and the materialisation of
   its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
