/* func_L00_003A6538 - 1784 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a6538. Screened with tools/w10_screen.py: 75 call(s)
   [func_002AFFD8@003A6590, func_00282CF0@003A65AC, func_00282F20@003A65BC,
   func_00283CE0@003A6614, func_00281DE8@003A6630, func_00283CF0@003A6644,
   func_00282D30@003A669C, func_00282CC0@003A66AC, func_002AB668@003A66BC,
   func_002ACDE8@003A66CC, func_00282DD8@003A66DC, func_00282F20@003A66EC,
   func_00282CC0@003A66FC, func_00282CF0@003A670C, func_00282D30@003A6724,
   func_00282CC0@003A6734, func_00282CC0@003A6744, func_00282CF0@003A6754,
   func_00280B90@003A6764, func_002AB668@003A6784, func_002ACDE8@003A6794,
   func_00283CE0@003A67C0, func_00283B98@003A67E0, func_00283B98@003A67F4,
   func_00281DE8@003A680C, func_00283CF0@003A681C, func_00282D30@003A6880,
   func_00282CC0@003A6890, func_00282CF0@003A68A0, func_00282F20@003A68B0,
   func_002AB668@003A68C0, func_00282DD8@003A68D0, func_00282DD8@003A68E0,
   func_002ACDE8@003A68F0, func_00282F20@003A6914, func_00282DD8@003A6924,
   func_00282F20@003A6934, func_00282CC0@003A6944, func_00282CC0@003A6954,
   func_00282CF0@003A6964, func_00282CF0@003A6974, func_00282CC0@003A6984,
   func_00282CF0@003A6994, func_00280B90@003A69A4, func_002AB668@003A69B4,
   func_002ACDE8@003A69C4, func_00282D30@003A69D4, func_00282DD8@003A69E4,
   func_00282F20@003A69F4, func_00282CC0@003A6A04, func_00282CC0@003A6A14,
   func_00282CF0@003A6A24, func_00282CF0@003A6A34, func_00282CC0@003A6A44,
   func_00282CF0@003A6A54, func_00280B90@003A6A64, func_00283CE0@003A6A78,
   func_00281DE8@003A6A94, func_00283CF0@003A6AA8, func_00282D30@003A6ADC,
   func_00282CC0@003A6AEC, func_00282DD8@003A6AFC, func_00282F20@003A6B0C,
   func_00282DD8@003A6B1C, func_00282F20@003A6B2C, func_00282CC0@003A6B3C,
   func_00282CC0@003A6B4C, func_00282CF0@003A6B5C, func_00282CF0@003A6B6C,
   func_00282CC0@003A6B7C, func_00282CF0@003A6B8C, func_00280B90@003A6B9C,
   func_00283CF0@003A6BA8, func_002FC560@003A6BCC, func_0027DE50@003A6BEC];
   data addresses materialised [0x1c0000, 0x200000, 0x3f490000, 0x1c0000,
   0x3c8e0000, 0x800000, 0x800000, 0xbdcc0000, 0x1a0000]; float constants
   [0x3f800000, 0x437f0000, 0x3f000000, 0x3f800000, 0x41a00000, 0xc1800000,
   0x41800000, 0x437f0000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   78620000@0x0044, LQ/SQ 7E420000@0x0048]. Head: addiu $sp, $sp, -0x1F0 ;
   sd $s2, 0x190($sp) ; sd $s0, 0x180($sp) ; addiu $s2, $sp, 0x90 ; sd $s1,
   0x188($sp) ; sd $s3, 0x198($sp). Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
