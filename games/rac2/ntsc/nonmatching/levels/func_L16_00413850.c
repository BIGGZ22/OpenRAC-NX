/* func_L16_00413850 - 1212 bytes, 2 place(s), level 16_snivelak, retail at
   0x00413850. Screened with tools/w10_screen.py: 34 call(s)
   [func_002A8C28@004138D4, func_002A7750@004138E4, func_00282D30@00413930,
   func_00282DF0@00413944, func_00282F20@00413964, func_00282F20@0041397C,
   func_00282CC0@0041398C, func_00282CC0@0041399C, func_002761F8@004139B4,
   func_00282F20@004139D8, func_00282F20@004139F4, func_00282D30@00413A18,
   func_00282F20@00413A28, func_00282DD8@00413A38, func_00282DB0@00413A44,
   func_00283198@00413A64, func_00282D30@00413A74, func_00283180@00413A7C,
   func_002837D0@00413ABC, func_002837D0@00413ACC, func_00282F20@00413ADC,
   func_00282D30@00413AF0, func_00282CC0@00413B08, func_002A8C00@00413B24,
   func_002761F8@00413B44, func_00277258@00413B68, func_00282D30@00413BBC,
   func_002A8C00@00413BF0, func_00277F48@00413C24, func_002A7750@00413C34,
   func_00282D30@00413C5C, func_0032C700@00413CA4, func_0029F338@00413CAC,
   func_L00_002E6EF8@00413CC0]; data addresses materialised [0x3f190000,
   0x10000, 0x410000, 0x3dcc0000, 0x1c0000, 0x1c0000, 0x3e990000, 0x1c0000,
   0x45af0000, 0x10000, 0x3f190000, 0x410000]; float constants [0xbf800000,
   0xc0400000, 0x3f800000, 0xbf800000, 0x3f800000, 0x3f000000, 0x3fc00000,
   0x40400000, 0xbf000000, 0x3e800000, 0x41a00000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [LQ/SQ 7A420000@0x00c0, LQ/SQ 7C820000@0x00c4, LQ/SQ
   7A420000@0x02a8, LQ/SQ 7EE20000@0x02ac, LQ/SQ 7A820000@0x0340, LQ/SQ
   7E020000@0x0344, LQ/SQ 7A820000@0x0358, LQ/SQ 7E020000@0x035c, LQ/SQ
   78620000@0x03f8, LQ/SQ 7EC20000@0x03fc]. Head: addiu $sp, $sp, -0x190 ;
   sd $s5, 0x138($sp) ; sd $s0, 0x110($sp) ; daddu $s5, $a0, $zero ; sd $s1,
   0x118($sp) ; sd $s2, 0x120($sp). Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
