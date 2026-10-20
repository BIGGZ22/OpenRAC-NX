/* func_L05_0037B538 - 292 bytes, 4 place(s), level 05_feltzin_system,
   retail at 0x0037b538. Screened with tools/w10_screen.py: 4 call(s)
   [func_002ED688@0037B560, func_00277258@0037B590, func_00282D30@0037B5F4,
   func_00282F20@0037B608]; data addresses materialised [0x3a7f0000,
   0x1c0000, 0x3a7f0000, 0x220000, 0x220000]; float constants [0x3f800000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 78620000@0x0088, LQ/SQ 7E020000@0x008c].
   Head: addiu $sp, $sp, -0x30 ; sd $s2, 0x10($sp) ; sd $s0, 0x0($sp) ;
   daddu $s2, $a0, $zero ; sd $s3, 0x18($sp) ; daddu $s0, $a3, $zero. Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
