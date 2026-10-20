/* func_L00_0032CF60 - 260 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0032cf60. Screened with tools/w10_screen.py: 2 call(s)
   [func_002A05A8@0032D010, func_002A0F48@0032D040]; data addresses
   materialised [0x200000, 0x1b0000, 0x1b0000]; float constants [0x1b0000];
   $gp-relative small data: yes   saves: -; callee-saved registers saved
   [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp, -0x10 ; lui
   $v0, 0x20 ; sw $zero, -0x5320($v0) ; sd $s0, 0x0($sp) ; addiu $v0, $zero,
   0xFF ; sd $ra, 0x8($sp). Not matched in this wave: the body is straight-
   line code whose register allocation and the materialisation of its
   constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
