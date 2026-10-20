/* func_L00_00398150 - 1728 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00398150. Screened with tools/w10_screen.py: 44 call(s)
   [func_002A7470@003981E8, func_002FE8D8@00398200, func_00282F20@0039822C,
   func_00282DD8@0039824C, func_00282DB0@00398258, func_00283198@00398268,
   func_00282D30@00398278, func_00283180@00398280, func_00283958@00398294,
   func_00283738@003982A4, func_00282CC0@003982C8, func_002FEE70@003982DC,
   func_002FE880@003982F4, func_00300190@00398300, func_00300248@0039830C,
   func_002FE9D0@003983A8, func_002ACB48@00398404, func_002ABBB8@00398474,
   func_00282D30@003984D8, func_00282F20@003984E8, func_00282F20@003984F8,
   func_00282DD8@00398508, func_00282DB0@00398514, func_00283198@0039852C,
   func_00282D30@0039853C, func_00283180@00398544, func_00283958@00398554,
   func_00283738@00398564, func_002A7470@0039858C, func_00282D30@003985B4,
   func_00300540@003985BC, func_002ACB48@003985D4, func_002ABBB8@00398648,
   func_002A7470@00398688, func_002A7878@003986A8, func_0030E8A8@003986B4,
   func_0030E700@003986BC, func_002FEE50@00398728, func_002ACAD0@00398748,
   func_0029F338@00398750, func_00283658@00398788, func_002AB6D8@00398798,
   func_002AD0E0@003987BC, func_002833F0@003987CC]; data addresses
   materialised [0x2a0000, 0x3b480000, 0x3e4c0000, 0x3ee60000, 0x3f0e0000,
   0x3e320000, 0x1b0000, 0x43e70000, 0x810000, 0x1c0000, 0x3f8c0000,
   0x810000, 0x3f330000, 0x3f860000, 0x3ca30000, 0x3dcc0000, 0x1b0000];
   float constants [0x3f800000, 0x3f000000, 0x40800000, 0x40400000,
   0x41100000, 0x41400000, 0x3a800000, 0x3f800000, 0x3f000000, 0x3f000000,
   0x40800000, 0x40400000, 0x41100000, 0x3f800000, 0x41400000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 7A220000@0x0160, LQ/SQ 7E420000@0x0164, LQ/SQ
   7A420000@0x01d4, LQ/SQ 7C820000@0x01d8, LQ/SQ 7A220000@0x01e0, LQ/SQ
   7E620000@0x01e4, LQ/SQ 7A020000@0x01ec, LQ/SQ 7E420000@0x01f0, LQ/SQ
   7A620000@0x0264, LQ/SQ 7E220000@0x0268, LQ/SQ 7A420000@0x026c, LQ/SQ
   7E020000@0x0270]. Head: addiu $sp, $sp, -0x1B0 ; sd $fp, 0x180($sp) ; sd
   $s0, 0x140($sp) ; daddu $fp, $a0, $zero ; sd $s1, 0x148($sp) ; sd $s2,
   0x150($sp). Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
