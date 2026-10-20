/* func_L11_002CDE10 - 1372 bytes, 2 place(s), level 11_joba, retail at
   0x002cde10. Screened with tools/w10_screen.py: 45 call(s)
   [func_L00_002D4140@002CDE5C, func_L00_002D3A58@002CDEA4,
   func_L00_002D3BD8@002CDED4, func_L00_002D3B00@002CDEE4,
   func_L00_002D3B58@002CDF2C, func_00282DF0@002CDF6C,
   func_00282F20@002CDFA0, func_00282CC0@002CDFB8, func_00282CC0@002CDFC8,
   func_00282C88@002CDFD0, func_00282DF0@002CDFE4,
   func_L00_002B5A88@002CE00C, func_L09_002B7238@002CE014,
   func_00282CF0@002CE024, func_L00_002B5250@002CE02C,
   func_L09_002B7238@002CE03C, func_00282CF0@002CE05C,
   func_00282F20@002CE090, func_00282DB0@002CE09C, func_00282F20@002CE0BC,
   func_L00_002D3960@002CE0D4, func_L00_002D3960@002CE0E0,
   func_00282F20@002CE0F0, func_00282DB0@002CE0FC, func_00282F20@002CE11C,
   func_L00_002D3BD8@002CE138, func_L00_002D3BD8@002CE148,
   func_00282F20@002CE158, func_00282DB0@002CE164, func_00282F20@002CE188,
   func_00282DF0@002CE190, func_L00_002D3A58@002CE19C,
   func_L00_002B5590@002CE1B0, func_L00_002D3B00@002CE1BC,
   func_00282DF0@002CE1EC, func_00282CC0@002CE218, func_00282C88@002CE224,
   func_L00_002B5A88@002CE22C, func_L09_002B7238@002CE234,
   func_00282CF0@002CE244, func_L00_002B5250@002CE24C,
   func_00282CF0@002CE260, func_00282DF0@002CE268, func_00282CF0@002CE288,
   func_00282D30@002CE330]; data addresses materialised [0x190000, 0x190000,
   0x3ca30000, 0x190000, 0x3ca30000, 0x190000, 0x3ca30000, 0x38d10000,
   0x190000, 0x190000, 0x38d10000, 0x3b830000, 0x3f5d0000]; float constants
   [0x3f800000, 0x3f000000, 0xbf000000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 7A220000@0x0034, LQ/SQ 7FA20000@0x0038, LQ/SQ 7A020000@0x0254,
   LQ/SQ 7E220000@0x0258, LQ/SQ 7A020000@0x0260, LQ/SQ 7E420000@0x0264,
   LQ/SQ 7A820000@0x02b8, LQ/SQ 7E620000@0x02bc, LQ/SQ 7A820000@0x0318,
   LQ/SQ 7E220000@0x031c, LQ/SQ 7A020000@0x0394, LQ/SQ 7FA20010@0x03a4,
   LQ/SQ 7AA20000@0x03cc, LQ/SQ 7E620000@0x03d0]. Head: addiu $sp, $sp,
   -0x80 ; lui $v1, 0x19 ; sd $s1, 0x38($sp) ; sd $s0, 0x30($sp) ; addiu
   $s1, $v1, -0x6160 ; sd $s2, 0x40($sp). Not matched in this wave: the
   128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
