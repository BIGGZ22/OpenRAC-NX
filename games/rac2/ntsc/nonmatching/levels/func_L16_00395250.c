/* func_L16_00395250 - 1544 bytes, 2 place(s), level 16_snivelak, retail at
   0x00395250. Screened with tools/w10_screen.py: 37 call(s)
   [func_002AFFD8@003952E8, func_L00_00325600@00395304,
   func_L00_00325600@00395314, func_002AB6D8@0039532C,
   func_002A7750@0039537C, func_00282F20@00395398, func_00282CC0@003953AC,
   func_00282948@003953B4, func_00282E48@003953CC, func_00288E18@00395448,
   func_L00_0032EF10@00395480, func_L16_003960F8@003954B4,
   func_002AFDD8@003954F0, func_00282CF0@00395504, func_002AFDD8@00395518,
   func_00282F20@00395534, func_00282F20@00395544, func_00282DB0@00395550,
   func_L16_002C0BC8@00395568, func_L16_00395D30@00395570,
   func_002A7750@00395580, func_00282C48@003955D4,
   func_L00_0032EF10@00395628, func_L16_002C0BC8@0039568C,
   func_003355A8@00395694, func_00288E18@003956C8, func_002A7750@00395720,
   func_00282F20@00395734, func_00282CC0@00395744,
   func_L16_00395858@0039574C, func_L16_003959B8@00395754,
   func_L16_00395C00@0039575C, func_L16_00395ED0@00395764,
   func_002B3868@00395774, func_00282C48@003957BC, func_00282F20@00395814,
   func_00282CC0@00395824]; data addresses materialised [0x1b0000, 0x1b0000,
   0x190000, 0x190000, 0x190000, 0x280000, 0x190000, 0x190000, 0x3e4c0000,
   0x3b360000, 0x3ada0000, 0xbeaa0000, 0x190000, 0x190000, 0x3b360000,
   0x3ada0000, 0xbeaa0000]; float constants [0x3f800000, 0x3fe00000,
   0x3f800000, 0x3f800000, 0x3f800000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7A220000@0x00d0, LQ/SQ 7C620000@0x00d4]. Head: addiu $sp, $sp, -0x80 ;
   addiu $v0, $zero, 0x1 ; sd $s3, 0x48($sp) ; sd $s0, 0x30($sp) ; daddu
   $s3, $a0, $zero ; sd $s1, 0x38($sp). Not matched in this wave: the
   128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
