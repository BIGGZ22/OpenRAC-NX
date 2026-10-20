/* func_L05_0037AFF8 - 272 bytes, 4 place(s), level 05_feltzin_system,
   retail at 0x0037aff8. Screened with tools/w10_screen.py: 3 call(s)
   [func_L00_004272A0@0037B03C, func_002ED688@0037B0BC,
   func_L05_0037AE40@0037B0D4]; data addresses materialised [0x200000,
   0x220000, 0x220000]; float constants [0x1b0000, 0x1b0000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [-]. Head: addiu $sp, $sp, -0x30 ; lui $v0, 0x20 ; sd $s3,
   0x18($sp) ; sd $s4, 0x20($sp) ; daddu $s3, $a0, $zero ; sd $s0, 0x0($sp).
   Not matched in this wave: the body is straight-line code whose register
   allocation and the materialisation of its constant addresses have to be
   reproduced word for word. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
