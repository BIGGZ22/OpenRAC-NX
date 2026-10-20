/* func_L11_003CF250 - 448 bytes, 2 place(s), level 11_joba, retail at
   0x003cf250. Screened with tools/w10_screen.py: 9 call(s)
   [func_002A7750@003CF29C, func_002B3868@003CF2AC,
   func_L00_00429020@003CF2C8, func_002B0CE0@003CF308,
   func_002ACBC8@003CF358, func_L11_002D8EF8@003CF368,
   func_00283B98@003CF388, func_00283198@003CF394, func_00283CF0@003CF3A4];
   data addresses materialised [0x190000, 0x190000, 0x1a0000, 0x3dd60000,
   0x1a0000, 0x80000000]; float constants [0x43000000, 0x80000000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: addiu $sp, $sp, -0x20 ; sd $s0,
   0x0($sp) ; sd $s1, 0x8($sp) ; daddu $s0, $a0, $zero ; sd $s2, 0x10($sp) ;
   sd $ra, 0x18($sp). Not matched in this wave: the body is straight-line
   code whose register allocation and the materialisation of its constant
   addresses have to be reproduced word for word. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
