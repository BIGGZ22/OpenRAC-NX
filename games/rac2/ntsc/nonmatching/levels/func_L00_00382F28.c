/* func_L00_00382F28 - 984 bytes, 3 place(s), level 00_aranos_tutorial,
   retail at 0x00382f28. Screened with tools/w10_screen.py: 15 call(s)
   [func_00282E48@00382FD4, func_00283CE0@00382FF4, func_00283CE0@00383014,
   func_00279BD8@00383030, func_00283CE0@00383040, func_00283CE0@0038305C,
   func_0027C878@003830AC, func_00282C48@00383124, func_00282C48@00383170,
   func_00283CF0@003831EC, func_00283CF0@0038322C, func_002FC560@00383244,
   func_00283CF0@00383260, func_00283CF0@00383270, func_0027E018@0038329C];
   data addresses materialised [0x1c0000, 0x1c0000, 0x1b0000, 0x1b0000,
   0xff0000]; float constants [0x3d800000, 0x1a0000, 0x3f800000, 0xc0a00000,
   0x1a0000, 0x40b00000, 0x1a0000, 0x1a0000, 0x40a00000, 0x3f800000,
   0x3f800000, 0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [COP1
   46806320@0x02bc, COP1 46800020@0x02e0, COP1 4680A520@0x0334]. Head: addiu
   $sp, $sp, -0xF0 ; lui $v0, 0x1C ; sd $s0, 0x60($sp) ; sd $s2, 0x70($sp) ;
   addiu $s0, $v0, -0x6F40 ; sd $s3, 0x78($sp). Not matched in this wave:
   the float code has to come out of the same expression tree to keep the
   FPU registers. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
