/* func_L00_003A2C60 - 636 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a2c60. Screened with tools/w10_screen.py: 18 call(s)
   [func_00282CF0@003A2CA4, func_00282CF0@003A2CE8, func_00282F20@003A2D00,
   func_00282DB0@003A2D0C, func_00282D30@003A2D1C, func_00282CF0@003A2D2C,
   func_002A7878@003A2D50, func_002A77E0@003A2D5C, func_002A7878@003A2D88,
   func_002ACEC0@003A2D9C, func_002A7878@003A2DA8, func_00282F20@003A2DB8,
   func_002A7878@003A2DC4, func_00282C88@003A2DD4, func_002C2210@003A2E10,
   func_00283CF0@003A2E2C, func_002C2210@003A2E7C, func_00283CF0@003A2E98];
   data addresses materialised [0x1c0000, 0x1b0000, 0x190000, 0x3c880000,
   0x3f860000, 0xbf860000, 0x3dcc0000, 0x7f7f0000, 0x3d8f0000, 0x7fff0000,
   0x3f330000]; float constants [0x3f800000, 0x3f800000, 0x40800000,
   0x40800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x90 ; sd $s2, 0x50($sp) ; sd $s3, 0x58($sp) ; daddu $s2, $a1, $zero ;
   daddu $s3, $a0, $zero ; sd $s4, 0x60($sp). Not matched in this wave: the
   body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
