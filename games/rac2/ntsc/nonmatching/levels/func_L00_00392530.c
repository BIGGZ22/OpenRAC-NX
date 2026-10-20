/* func_L00_00392530 - 2644 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00392530. Screened with tools/w10_screen.py: 42 call(s)
   [func_L00_002D4040@003925D0, func_00282948@003925EC,
   func_L00_002D4040@00392604, func_00273318@00392644,
   func_00282978@003926D0, func_00282978@003926E0, func_00282978@003926E8,
   func_L00_002D3F30@003926F4, func_00291308@0039272C,
   func_00283248@00392774, func_00283C80@00392780, func_00282E48@003927B8,
   func_00283248@00392804, func_00283C80@00392810, func_00282E48@0039284C,
   func_002AFFD8@00392880, func_00282D30@003928A4, func_00282CC0@003928B4,
   func_002AB1F0@003928E8, func_002AB1F0@00392944, func_0029FCC0@003929F0,
   func_00283098@00392A18, func_00282CC0@00392A28, func_0029FCC0@00392A3C,
   func_00282F20@00392A4C, func_00282CC0@00392A5C, func_0030A188@00392A64,
   func_L00_00393148@00392A6C, func_L00_002E6EF8@00392A7C,
   func_002E59F8@00392B0C, func_002E59B0@00392B1C, func_002E5CF8@00392B34,
   func_00291308@00392BEC, func_00282948@00392C38,
   func_L00_002AF568@00392C48, func_L00_002AF568@00392CE0,
   func_002E5CF8@00392D10, func_L00_002AF7D0@00392D18,
   func_L00_002AF7D0@00392DFC, func_00283098@00392E40,
   func_00283CE0@00392EC0, func_00291450@00392F30]; data addresses
   materialised [0x190000, 0x140000, 0x3f4c0000, 0x3f660000, 0x260000,
   0x600f0000, 0x60000000, 0x40060000, 0x40060000, 0x3f330000, 0x140000,
   0x140000, 0x260000, 0x3d8f0000, 0x390000, 0x190000, 0x2a0000, 0x190000,
   0x1a0000, 0x1a0000, 0x80000000, 0x1a0000, 0x1a0000, 0x80000000, 0x190000,
   0x1c0000, 0x3dcc0000, 0x3f330000]; float constants [0x3f800000,
   0x42b40000, 0x40f00000, 0x41a00000, 0x41a00000, 0x41400000, 0x41900000,
   0x190000, 0x40000000, 0x40f00000]; $gp-relative small data: yes   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7A220000@0x0358, LQ/SQ 7E420000@0x035c, LQ/SQ 7A420000@0x0390, LQ/SQ
   7C620000@0x0394, LQ/SQ 7FA00000@0x08ec]. Head: addiu $sp, $sp, -0x80 ;
   lui $v1, 0x19 ; sd $s3, 0x48($sp) ; lui $v0, 0x14 ; sd $s4, 0x50($sp) ;
   addiu $a3, $v1, -0x61E0. Not matched in this wave: the 128-bit copy needs
   an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local,
   and this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
