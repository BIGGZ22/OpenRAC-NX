/* func_L02_00422D68 - 840 bytes, 2 place(s), level 02_maktar_nebula, retail
   at 0x00422d68. Screened with tools/w10_screen.py: 26 call(s)
   [func_L00_003DA2B0@00422DB8, func_002A7878@00422DE0,
   func_00282D08@00422E00, func_002A7878@00422E14, func_002A7878@00422E34,
   func_002A7878@00422E54, func_0027C878@00422E70, 0x1163b0@00422E8C,
   func_002A7820@00422EC8, func_00283248@00422F10, func_00282E80@00422F20,
   func_00283248@00422F34, func_002A7940@00422F40, func_00283410@00422F54,
   func_002A7878@00422F8C, func_00282D30@00422FA4, func_00282D30@00422FB8,
   func_00282D30@00422FCC, func_00282D30@00422FE0, func_00280B90@00422FF0,
   func_00282D30@0042301C, func_00282D30@0042302C, func_00282D30@0042303C,
   func_00282D30@0042304C, func_00280B90@0042305C,
   func_L02_004230B0@00423068]; data addresses materialised [0xff0000,
   0xbdcc0000, 0x3dcc0000, 0x1c0000, 0x3e190000, 0x3d4c0000]; float
   constants [0x3f800000, 0x3f000000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [-]. Head:
   addiu $sp, $sp, -0x170 ; sd $s0, 0xF0($sp) ; sd $s2, 0x100($sp) ; sd $s3,
   0x108($sp) ; sd $s4, 0x110($sp) ; sd $s5, 0x118($sp). Not matched in this
   wave: the body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
