/* func_L05_00379968 - 504 bytes, 4 place(s), level 05_feltzin_system,
   retail at 0x00379968. Screened with tools/w10_screen.py: 0 call(s) [-];
   data addresses materialised [0x1a0000, 0x2a0000, 0x1a0000, 0x1a0000,
   0x1a0000, 0x1a0000, 0x1a0000, 0x1a0000]; float constants [0x1a0000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: lui $v0, 0x1A ; lw $v0, 0x7AF0($v0)
   ; andi $v0, $v0, 0x40 ; beqz $v0, . + 4 + (0x8 << 2) ; lui $v0, 0x2A ;
   addiu $a0, $zero, 0x1. Not matched in this wave: the body is straight-
   line code whose register allocation and the materialisation of its
   constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
