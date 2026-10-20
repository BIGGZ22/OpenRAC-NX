/* func_L18_003FB500 - 232 bytes, 3 place(s), level 18_damosel, retail at
   0x003fb500. Screened with tools/w10_screen.py: 3 call(s)
   [func_002A7750@003FB528, func_002ABBB8@003FB5AC, func_002E5DA0@003FB5C0];
   data addresses materialised [0x810000, 0x409c0000, 0x1b0000]; float
   constants [0x40a00000, 0x42000000, 0x40000000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [-]. Head: addiu $sp, $sp, -0x40 ; addiu $v0, $zero, 0x2 ; sd $s0,
   0x20($sp) ; sd $s1, 0x28($sp) ; daddu $s0, $a0, $zero ; sd $ra,
   0x30($sp). Not matched in this wave: the body is straight-line code whose
   register allocation and the materialisation of its constant addresses
   have to be reproduced word for word. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
