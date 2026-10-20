/* func_L18_003FA7C0 - 288 bytes, 3 place(s), level 18_damosel, retail at
   0x003fa7c0. Screened with tools/w10_screen.py: 0 call(s) [-]; data
   addresses materialised [0x200000, 0x1b0000, 0x1b0000, 0x1b0000]; float
   constants [-]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x10 ; lui $v0, 0x20 ; sd $s0, 0x0($sp) ; addiu $t9, $v0, -0x4CE0 ; lui
   $t7, 0x1B ; lw $t7, 0x2838($t7). Not matched in this wave: the body is
   straight-line code whose register allocation and the materialisation of
   its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
