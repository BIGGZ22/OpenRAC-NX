/* func_L07_003F6B88 - 184 bytes, 2 place(s), level 07_siberius, retail at
   0x003f6b88. Screened with tools/w10_screen.py: 0 call(s) [-]; data
   addresses materialised [0x200000, 0x1b0000, 0x1b0000]; float constants
   [-]; $gp-relative small data: no   saves: -; callee-saved registers saved
   [-]; words plain C cannot emit [-]. Head: daddu $a2, $a0, $zero ; lui
   $v0, 0x20 ; addiu $v0, $v0, -0x52A0 ; sll $v1, $a2, 2 ; addu $v1, $v1,
   $v0 ; andi $a3, $a2, 0xFF. Not matched in this wave: the body is
   straight-line code whose register allocation and the materialisation of
   its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
