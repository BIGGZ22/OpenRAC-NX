/* func_L00_0038BA80 - 640 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038ba80. Screened with tools/w10_screen.py: 24 call(s)
   [func_00282CF0@0038BACC, func_00282E20@0038BAE4, func_00283248@0038BAF0,
   func_00283248@0038BB00, func_002A78D8@0038BB18, func_00283B98@0038BB28,
   func_002A78D8@0038BB38, func_00283B98@0038BB44, func_002AF000@0038BB60,
   func_002A7878@0038BB78, func_00282D30@0038BB88, func_00282CC0@0038BB98,
   func_L00_00333F90@0038BBA4, func_002A78D8@0038BBD0,
   func_002A78D8@0038BBEC, func_002A78D8@0038BC04, func_002A78D8@0038BC1C,
   func_00283B98@0038BC28, func_002A78D8@0038BC38, func_00283B98@0038BC44,
   func_002AF000@0038BC60, func_002A7820@0038BC6C, func_002BD8B0@0038BC9C,
   func_L00_0038B888@0038BCC4]; data addresses materialised [0x1c0000,
   0x1c0000, 0x40490000, 0x3e4c0000, 0x1c0000, 0x3dcc0000, 0x3d4c0000,
   0x4f000000, 0x1fff0000, 0x469c0000]; float constants [0x40400000,
   0x3f800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x80 ; lui $a2, 0x1C ; sd $s3, 0x38($sp) ; lui $a1, 0x1C ; sd $s4,
   0x40($sp) ; daddu $s3, $a2, $zero. Not matched in this wave: the body is
   straight-line code whose register allocation and the materialisation of
   its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
