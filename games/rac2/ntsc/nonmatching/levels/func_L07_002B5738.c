/* func_L07_002B5738 - 1972 bytes, 4 place(s), level 07_siberius, retail at
   0x002b5738. Screened with tools/w10_screen.py: 42 call(s)
   [func_L00_002B54F8@002B577C, func_002AA2E8@002B57D4,
   func_L00_002B41B8@002B5818, func_L00_002D3BD8@002B5830,
   func_L00_002D3798@002B5840, func_L00_002B3EF8@002B588C,
   func_L01_002B6948@002B58A0, func_002AA2E8@002B5910,
   func_002AA2E8@002B5938, func_00282DB0@002B5958, func_00283098@002B596C,
   func_L00_002B3EF8@002B59C0, func_002AA2E8@002B5A2C,
   func_002AA2E8@002B5A68, func_002AA2E8@002B5A90, func_00283180@002B5AA8,
   func_00283198@002B5ABC, func_L00_002D3BD8@002B5ADC,
   func_L00_002D3B58@002B5AF0, func_L00_002D3BD8@002B5B00,
   func_00282CC0@002B5B10, func_00283C80@002B5B38, func_00283198@002B5B40,
   func_00283180@002B5B50, func_00283198@002B5B68,
   func_L00_002B54F8@002B5B90, func_L00_002B3EF8@002B5C08,
   func_00283180@002B5C38, func_00283198@002B5C48,
   func_L00_002D3BD8@002B5C64, func_00282F20@002B5C7C,
   func_00282CC0@002B5C8C, func_L00_002B3EF8@002B5CD8,
   func_002AA898@002B5D30, func_00283BE0@002B5D40,
   func_L00_002AA0F0@002B5D50, func_L00_002B3EF8@002B5DAC,
   func_L00_002B54F8@002B5DE8, func_L00_002D3A58@002B5E54,
   func_002AA2E8@002B5E68, func_L00_002D3BD8@002B5E78,
   func_L01_002B6948@002B5EC4]; data addresses materialised [0x190000,
   0x3f330000, 0x3be30000, 0x3afe0000, 0x3d6e0000, 0x3c480000, 0x3c4c0000,
   0x3d230000, 0x3e4c0000, 0x3e800000, 0x3bb60000, 0x3b110000, 0x3d080000,
   0x3b110000, 0x3d230000, 0x3e4c0000, 0x3dee0000, 0x3daa0000, 0x3d080000,
   0x3c280000, 0x190000, 0x3b110000, 0x3d8f0000, 0x3b110000, 0x3f330000,
   0x190000, 0x3d2c0000, 0x3e4c0000, 0x3e4a0000, 0x3da70000, 0x3da70000,
   0x1b0000, 0x3d230000, 0x3e4c0000, 0x3e4c0000, 0x3c930000, 0x3dfa0000,
   0x1c0000, 0x3d230000, 0x3e4c0000, 0x3f330000, 0x190000, 0x1a0000,
   0x3b5a0000, 0x3bb60000, 0x3b480000, 0x3b110000, 0x3a110000]; float
   constants [0x190000, 0x41500000, 0x41f00000, 0x41d80000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 78820000@0x0054, LQ/SQ 7C620000@0x0058, LQ/SQ
   78620000@0x025c, LQ/SQ 7C820000@0x0260, LQ/SQ 78820000@0x046c, LQ/SQ
   7C620000@0x0470, LQ/SQ 78820000@0x0494, LQ/SQ 7E220000@0x0498, LQ/SQ
   7A220000@0x062c, LQ/SQ 7FA20000@0x0630, LQ/SQ 78620000@0x0650, LQ/SQ
   7E220000@0x0654, LQ/SQ 7BA20000@0x068c, LQ/SQ 7C620000@0x0690, LQ/SQ
   78620000@0x0698, LQ/SQ 7E220000@0x069c, LQ/SQ 78620000@0x06c4, LQ/SQ
   7E020000@0x06c8]. Head: addiu $sp, $sp, -0x50 ; lui $v0, 0x19 ; sd $s0,
   0x20($sp) ; addiu $v1, $zero, 0xA ; sd $s2, 0x30($sp) ; addiu $s0, $v0,
   -0x61E0. Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
