/* func_L11_0046C118 - 200 bytes, 2 place(s), level 11_joba, retail at
   0x0046c118. Screened with tools/w10_screen.py: 2 call(s)
   [func_L11_0046C1E0@0046C194, func_L11_0046C1E0@0046C1C0]; data addresses
   materialised [-]; float constants [0xc1000000, 0x41000000, 0x260000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: lwc1 $f0, 0x3610($gp) ; addiu $sp,
   $sp, -0x20 ; lwc1 $f1, 0x3614($gp) ; lui $at, 0xC100 ; mtc1 $at, $f2 ;
   sub.s $f1, $f0, $f1. Not matched in this wave: the body is straight-line
   code whose register allocation and the materialisation of its constant
   addresses have to be reproduced word for word. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
