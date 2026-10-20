/* func_L18_00426E48 - 224 bytes, 3 place(s), level 18_damosel, retail at
   0x00426e48. Screened with tools/w10_screen.py: 3 call(s)
   [func_00282E80@00426EB4, func_00283248@00426ED4, func_00283C80@00426EE0];
   data addresses materialised [-]; float constants [-]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [-]. Head: addiu $sp, $sp, -0x40 ; addiu $v0, $zero, 0xED4 ;
   sd $s0, 0x0($sp) ; sd $s1, 0x8($sp) ; daddu $s0, $a0, $zero ; swc1 $f22,
   0x30($sp). Not matched in this wave: the body is straight-line code whose
   register allocation and the materialisation of its constant addresses
   have to be reproduced word for word. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
