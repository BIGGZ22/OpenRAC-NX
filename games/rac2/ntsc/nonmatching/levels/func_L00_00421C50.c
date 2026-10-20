/* func_L00_00421C50 - 1560 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00421c50. Screened with tools/w10_screen.py: 53 call(s)
   [func_00334710@00421CA8, func_L00_00420C38@00421CB0,
   func_L00_004212B8@00421CBC, func_00282DB0@00421CE4,
   func_00282D30@00421CFC, func_00282CF0@00421D10, func_00282E48@00421D1C,
   func_002702C8@00421D3C, func_002702C8@00421D5C, func_002702C8@00421D7C,
   func_002702C8@00421D9C, func_002702C8@00421DBC, func_002702C8@00421DDC,
   func_00282CC0@00421DF0, func_00282CC0@00421E00, func_00282D30@00421E14,
   func_00283CE0@00421E50, func_00283CE0@00421E5C, func_00282D08@00421E80,
   func_00282CC0@00421E90, func_00282978@00421EA0, func_00283CE0@00421EB4,
   func_00283CE0@00421EC0, func_00282D08@00421EE4, func_00282CC0@00421EF4,
   func_L00_00421B50@00421F00, func_00282D30@00421F30,
   func_00282D30@00421F5C, func_00282D30@00421F90, func_00282CC0@00421FA8,
   func_00282D30@00421FC0, func_00282CC0@00421FD0, func_00282D30@00422020,
   func_00282CC0@00422030, func_002761F8@00422054, func_00282D30@00422098,
   func_00282CF0@004220A8, func_002761F8@004220CC, func_00282CF0@004220F4,
   func_00282DF0@004220FC, func_00282CC0@00422120, func_00282D30@00422134,
   func_00282CF0@0042214C, func_00282DF0@00422154, func_00282D30@00422180,
   func_00282CF0@00422190, func_00282CF0@004221A8, func_00282DB0@004221B4,
   func_L00_00424A50@004221C8, func_00282DB0@004221E0,
   func_00282DB0@004221F0, func_00282D30@00422218, func_00282CC0@00422228];
   data addresses materialised [0x1c0000, 0x190000, 0x3ecc0000, 0x1c0000,
   0x190000, 0x1b0000, 0x1c0000, 0x190000, 0x1b0000, 0x1c0000, 0x3bc40000];
   float constants [0x40400000, 0x3f000000, 0x3f800000, 0x41a00000,
   0x41a00000, 0x3f000000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7BA20000@0x0074, LQ/SQ 7E420000@0x0078, LQ/SQ 7A620000@0x0080, LQ/SQ
   7C620000@0x0084, LQ/SQ 7EC00000@0x0328, LQ/SQ 78620000@0x041c, LQ/SQ
   7C820000@0x0420, LQ/SQ 7A820000@0x0430, LQ/SQ 7C620000@0x0434, LQ/SQ
   78620000@0x0490, LQ/SQ 7E820000@0x0494, LQ/SQ 7A020000@0x0548, LQ/SQ
   7EA20000@0x054c, LQ/SQ 7A620000@0x05b8, LQ/SQ 7E820000@0x05bc]. Head:
   addiu $sp, $sp, -0x120 ; lui $v0, 0x1C ; sd $s0, 0xC0($sp) ; sd $s2,
   0xD0($sp) ; addiu $s0, $sp, 0x10 ; sd $s3, 0xD8($sp). Not matched in this
   wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only
   through a `mode(TI)` local, and this cc1 folds the destination offset
   into the `sq` where retail materialises the pointer first. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
