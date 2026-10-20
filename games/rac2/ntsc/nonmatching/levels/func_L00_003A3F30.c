/* func_L00_003A3F30 - 700 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a3f30. Screened with tools/w10_screen.py: 13 call(s)
   [func_002A75E0@003A3F90, func_L00_0037E000@003A3FA4,
   func_L00_002C42E0@003A3FE8, func_L00_002D4040@003A401C,
   func_00273318@003A405C, func_00282948@003A4084, func_002829D8@003A409C,
   func_L00_002D3F30@003A40EC, func_0029FCC0@003A410C,
   func_002E5CF8@003A411C, func_002A75E0@003A4134,
   func_L00_003AB8B8@003A4140, func_002E5CF8@003A4188]; data addresses
   materialised [0x190000, 0x190000, 0x260000, 0x3dcc0000, 0x600f0000,
   0x60000000, 0x600f0000, 0x140000, 0x1b0000, 0x140000, 0x1b0000, 0x140000,
   0x1b0000, 0x140000, 0x260000]; float constants [0x40000000, 0x42b40000,
   0x3f800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x40 ; sd $s0, 0x10($sp) ; sd $s1, 0x18($sp) ; daddu $s0, $a0, $zero ;
   sd $s2, 0x20($sp) ; sd $s3, 0x28($sp). Not matched in this wave: the body
   is straight-line code whose register allocation and the materialisation
   of its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
