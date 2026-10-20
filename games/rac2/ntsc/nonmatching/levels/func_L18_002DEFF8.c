/* func_L18_002DEFF8 - 2016 bytes, 2 place(s), level 18_damosel, retail at
   0x002deff8. Screened with tools/w10_screen.py: 55 call(s)
   [func_L00_002B9380@002DF038, func_00282978@002DF070,
   func_00282978@002DF078, func_00282978@002DF080, func_00282978@002DF088,
   func_00282978@002DF090, func_00282978@002DF098, func_00282948@002DF0A0,
   func_00282978@002DF0A8, func_00282978@002DF0B0, func_00282978@002DF0B8,
   func_00282978@002DF0C0, func_00282978@002DF0C8, func_00282978@002DF0D0,
   func_00282978@002DF0D8, func_00282978@002DF0E0, func_00282978@002DF0E8,
   func_00282978@002DF0F0, func_00282978@002DF0F8, func_00282978@002DF100,
   func_00282978@002DF108, func_00282978@002DF110, func_00282978@002DF118,
   func_00282978@002DF120, func_00282978@002DF128, func_00282978@002DF130,
   func_00282978@002DF138, func_00282978@002DF140, func_00282978@002DF148,
   func_00282978@002DF150, func_00282978@002DF158, func_00282978@002DF160,
   func_00282978@002DF168, func_00282978@002DF170, func_00282978@002DF178,
   func_L00_002A7A10@002DF1A8, func_L18_002E0070@002DF2D8,
   func_002ABB78@002DF3C8, func_002A77E0@002DF414, func_00282E48@002DF430,
   func_00282CF0@002DF44C, func_00282F20@002DF464, func_002A8F70@002DF484,
   func_00282948@002DF4A8, func_L00_002D3B00@002DF4E0,
   func_00282C88@002DF50C, func_00282CF0@002DF574, func_00282CC0@002DF588,
   func_00282D30@002DF5BC, func_00282DF0@002DF5C4,
   func_L00_002C3838@002DF604, func_L00_002D36E0@002DF634,
   func_L00_002D36E0@002DF658, func_L00_002D3778@002DF714,
   func_L00_002B30E8@002DF7A0]; data addresses materialised [0x190000,
   0x140000, 0x3ada0000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000,
   0x200000, 0x260000, 0x41750000, 0x190000, 0x1b0000, 0x3fd90000,
   0x4000000, 0x260000, 0x461c0000, 0x3c880000, 0x3ecc0000, 0x3f330000,
   0x1b0000, 0x1a0000]; float constants [0x3f800000, 0x3f800000, 0x40080000,
   0x3fa00000, 0x41600000, 0x40900000, 0x40880000, 0x40100000, 0x40800000,
   0x3f000000, 0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   78820000@0x0268, LQ/SQ 7C620000@0x026c, COP1 46800020@0x05b8, LQ/SQ
   78620000@0x070c, LQ/SQ 7C820000@0x0710, LQ/SQ 78820000@0x0734, LQ/SQ
   7C620000@0x0738]. Head: addiu $sp, $sp, -0x80 ; lui $v1, 0x19 ; sd $s0,
   0x20($sp) ; sd $s5, 0x48($sp) ; addiu $s0, $v1, -0x61E0 ; sd $s1,
   0x28($sp). Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
