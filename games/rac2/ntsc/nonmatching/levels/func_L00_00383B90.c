/* func_L00_00383B90 - 1248 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00383b90. Screened with tools/w10_screen.py: 33 call(s)
   [func_002B49E0@00383C28, func_L00_00384070@00383C44,
   func_00283360@00383C54, func_002A7750@00383C64,
   func_L00_002E6CF0@00383CA8, func_002AA2E8@00383CC8,
   func_002AA2E8@00383CDC, func_L00_00384248@00383CE4,
   func_L00_00384678@00383CEC, func_L00_00384F28@00383CF4,
   func_L00_00385040@00383CFC, func_L00_00384C20@00383D18,
   func_002B4B70@00383D64, func_00282CC0@00383D80,
   func_L00_00384E08@00383D8C, func_00277F48@00383DAC,
   func_00282CF0@00383DD0, func_002AFDD8@00383DE4, func_002A8F70@00383E30,
   func_0029F338@00383E60, func_L00_003851A0@00383EA4,
   func_002E5CF8@00383ECC, func_002AA340@00383EE0, func_0029F338@00383EE8,
   func_00283B98@00383F1C, func_00282CC0@00383F30, func_00283698@00383F48,
   func_00282CF0@00383F58, func_00283098@00383F68, func_00282C48@00383F7C,
   func_00282C48@00383FD4, func_002A0F48@00383FFC, func_0029F338@0038403C];
   data addresses materialised [0x1b0000, 0x1b0000, 0x1b0000, 0x1c0000,
   0x380000, 0x1b0000, 0x3e080000, 0x1b0000, 0x140000, 0x140000, 0x1b0000,
   0x3c230000, 0x1c0000, 0x1b0000, 0x10000, 0x1b0000, 0x1b0000, 0x1b0000,
   0xbd560000, 0x1c0000, 0x3ca30000, 0x3d380000, 0x3d4c0000, 0x1b0000];
   float constants [0x1b0000, 0x1b0000, 0x3f800000, 0x3f000000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: addiu $sp, $sp, -0xA0 ; addiu $v0,
   $zero, 0x1 ; sd $s3, 0x68($sp) ; sd $s0, 0x50($sp) ; daddu $s3, $a0,
   $zero ; sd $s1, 0x58($sp). Not matched in this wave: the body is
   straight-line code whose register allocation and the materialisation of
   its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
