/* func_L05_0037B280 - 284 bytes, 4 place(s), level 05_feltzin_system,
   retail at 0x0037b280. Screened with tools/w10_screen.py: 3 call(s)
   [func_L00_004272A0@0037B2C8, func_002ED688@0037B34C,
   func_L05_0037AEC0@0037B364]; data addresses materialised [0x200000,
   0x220000, 0x220000]; float constants [0x1b0000, 0x1b0000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [-]. Head: addiu $sp, $sp, -0x40 ; lui $v0, 0x20 ; sd $s3,
   0x18($sp) ; sd $s4, 0x20($sp) ; daddu $s3, $a0, $zero ; sd $s5,
   0x28($sp). Not matched in this wave: the body is straight-line code whose
   register allocation and the materialisation of its constant addresses
   have to be reproduced word for word. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
