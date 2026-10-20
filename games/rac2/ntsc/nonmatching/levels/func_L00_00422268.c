/* func_L00_00422268 - 612 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00422268. Screened with tools/w10_screen.py: 5 call(s)
   [func_002702C8@004222F8, func_002702C8@0042232C, func_002702C8@00422368,
   func_002AA548@004223F0, func_002AA548@0042242C]; data addresses
   materialised [0x1b0000, 0x1c0000, 0x3e4c0000, 0x3ba30000, 0x3e4c0000,
   0x3e4c0000, 0x1b0000]; float constants [0x3f800000, 0x3f800000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: addiu $sp, $sp, -0x30 ; lui $at,
   0x1B ; lwc1 $f0, -0x76FC($at) ; sd $s0, 0x0($sp) ; sd $s1, 0x8($sp) ; sd
   $s2, 0x10($sp). Not matched in this wave: the body is straight-line code
   whose register allocation and the materialisation of its constant
   addresses have to be reproduced word for word. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
