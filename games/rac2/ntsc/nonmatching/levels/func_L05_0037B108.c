/* func_L05_0037B108 - 264 bytes, 4 place(s), level 05_feltzin_system,
   retail at 0x0037b108. Screened with tools/w10_screen.py: 1 call(s)
   [func_0029F338@0037B1CC]; data addresses materialised [0x1b0000]; float
   constants [-]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x50 ; sd $s0, 0x0($sp) ; lui $s0, 0x1B ; lw $s0, 0x2528($s0) ; sd $s1,
   0x8($sp) ; sd $s2, 0x10($sp). Not matched in this wave: the body is
   straight-line code whose register allocation and the materialisation of
   its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
