/* func_L00_003A41F0 - 656 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a41f0. Screened with tools/w10_screen.py: 13 call(s)
   [func_002829A8@003A4238, func_00282D30@003A4258, func_00282CC0@003A4270,
   func_00282D30@003A428C, func_00282CC0@003A429C, func_00282D30@003A42B8,
   func_002A77E0@003A42C0, func_002A77E0@003A42CC, func_002BDCE8@003A4318,
   func_00283CF0@003A4338, func_002BDCE8@003A43D4, func_00283CF0@003A43F0,
   func_002A77E0@003A4414]; data addresses materialised [0x3dcc0000,
   0x3e8c0000, 0x3dcc0000, 0x3d230000, 0x7f200000, 0x3f660000, 0x474d0000,
   0x46760000, 0x45440000, 0x3ca30000, 0x7fff0000]; float constants
   [0x3f800000, 0x40800000, 0x3f800000, 0x40800000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [-]. Head: addiu $sp, $sp, -0xA0 ; sd $s3, 0x48($sp) ; sd
   $s0, 0x30($sp) ; daddu $s3, $a0, $zero ; sd $s1, 0x38($sp) ; sd $s2,
   0x40($sp). Not matched in this wave: the body is straight-line code whose
   register allocation and the materialisation of its constant addresses
   have to be reproduced word for word. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
