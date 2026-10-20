/* func_L00_004200F0 - 544 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x004200f0. Screened with tools/w10_screen.py: 15 call(s)
   [func_00282CF0@00420148, func_00283248@00420168, func_002A7878@004201BC,
   func_002A78D8@004201D0, func_00283CF0@004201D8, func_002A7878@004201E8,
   func_002A7878@004201F4, func_002A78D8@00420204, func_00283B98@00420210,
   func_002AF000@00420230, func_002A78D8@0042023C, func_002A78D8@00420254,
   func_002A78D8@0042026C, func_002A7878@00420284, func_002C16C0@004202BC];
   data addresses materialised [0x1c0000, 0x3d4c0000, 0x3e4c0000,
   0x484d0000, 0x3fb20000, 0x3dcc0000, 0x3c880000, 0x7f0000]; float
   constants [0x3f800000, 0x3f000000, 0x40000000, 0x43fa0000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [-]. Head: addiu $sp, $sp, -0xB0 ; lui $a1, 0x1C ; sd $s1,
   0x38($sp) ; addiu $a1, $a1, -0x7240 ; daddu $s1, $a0, $zero ; sd $s2,
   0x40($sp). Not matched in this wave: the body is straight-line code whose
   register allocation and the materialisation of its constant addresses
   have to be reproduced word for word. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
