/* func_L16_002B5EE0 - 1456 bytes, 2 place(s), level 16_snivelak, retail at
   0x002b5ee0. Screened with tools/w10_screen.py: 47 call(s)
   [func_L00_002D4140@002B5F2C, func_L00_002D3A58@002B5F74,
   func_L00_002D3BD8@002B5FA4, func_L00_002D3B00@002B5FB4,
   func_L00_002D3B58@002B5FFC, func_00282E20@002B6028,
   func_00282F70@002B605C, func_00282DF0@002B6090, func_00282F20@002B60C4,
   func_00282CC0@002B60DC, func_00282CC0@002B60EC, func_00282C88@002B60F4,
   func_00282DF0@002B6108, func_L00_002B5A88@002B6130,
   func_L09_002B7238@002B6138, func_00282CF0@002B6148,
   func_L00_002B5250@002B6150, func_L09_002B7238@002B6160,
   func_00282CF0@002B6180, func_00282F20@002B61B4, func_00282DB0@002B61C0,
   func_00282F20@002B61E0, func_L00_002D3960@002B61F8,
   func_L00_002D3960@002B6204, func_00282F20@002B6214,
   func_00282DB0@002B6220, func_00282F20@002B6240,
   func_L00_002D3BD8@002B625C, func_L00_002D3BD8@002B626C,
   func_00282F20@002B627C, func_00282DB0@002B6288, func_00282F20@002B62AC,
   func_00282DF0@002B62B4, func_L00_002D3A58@002B62C0,
   func_L00_002B5590@002B62D4, func_L00_002D3B00@002B62E0,
   func_00282DF0@002B6310, func_00282CC0@002B633C, func_00282C88@002B6348,
   func_L00_002B5A88@002B6350, func_L09_002B7238@002B6358,
   func_00282CF0@002B6368, func_L00_002B5250@002B6370,
   func_00282CF0@002B6384, func_00282DF0@002B638C, func_00282CF0@002B63AC,
   func_00282D30@002B6454]; data addresses materialised [0x190000, 0x190000,
   0x3ca30000, 0x190000, 0x3ca30000, 0x190000, 0x3ca30000, 0x190000,
   0x3ca30000, 0x38d10000, 0x190000, 0x190000, 0x38d10000, 0x3b830000,
   0x3f5d0000]; float constants [0x3f800000, 0x3f000000, 0xbf000000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 7A420000@0x0034, LQ/SQ 7FA20000@0x0038,
   LQ/SQ 7A020000@0x02a8, LQ/SQ 7E220000@0x02ac, LQ/SQ 7A020000@0x02b4,
   LQ/SQ 7E420000@0x02b8, LQ/SQ 7A820000@0x030c, LQ/SQ 7E620000@0x0310,
   LQ/SQ 7A820000@0x036c, LQ/SQ 7E220000@0x0370, LQ/SQ 7A020000@0x03e8,
   LQ/SQ 7FA20010@0x03f8, LQ/SQ 7AA20000@0x0420, LQ/SQ 7E620000@0x0424].
   Head: addiu $sp, $sp, -0x80 ; lui $v1, 0x19 ; sd $s2, 0x40($sp) ; sd $s0,
   0x30($sp) ; addiu $s2, $v1, -0x6160 ; sd $s1, 0x38($sp). Not matched in
   this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it
   only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
