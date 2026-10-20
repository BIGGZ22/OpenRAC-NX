/* func_L00_003A2408 - 356 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a2408. Screened with tools/w10_screen.py: 10 call(s)
   [func_00283CE0@003A2450, func_00283B98@003A2488, func_00283180@003A24A0,
   func_00283198@003A24B0, func_00283098@003A24CC, func_00282CC0@003A24E0,
   func_00282CF0@003A24F8, func_00282CF0@003A2508, func_00282DD8@003A2518,
   func_00282F20@003A252C]; data addresses materialised [0x40c90000]; float
   constants [0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [-]. Head: addiu
   $sp, $sp, -0x160 ; sd $s3, 0x108($sp) ; sd $s1, 0xF8($sp) ; daddu $s3,
   $a0, $zero ; sd $s6, 0x120($sp) ; daddu $s1, $a1, $zero. Not matched in
   this wave: the body is straight-line code whose register allocation and
   the materialisation of its constant addresses have to be reproduced word
   for word. What would unblock it: the exact source expression tree, so
   this cc1 picks the same registers in the same order. */
