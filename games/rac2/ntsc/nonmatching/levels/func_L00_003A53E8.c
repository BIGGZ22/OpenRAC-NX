/* func_L00_003A53E8 - 2516 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a53e8. Screened with tools/w10_screen.py: 61 call(s)
   [func_00282978@003A5454, func_002E5CF8@003A5468, func_002A75E0@003A54A4,
   func_L00_002C42E0@003A54C4, func_002829D8@003A54CC,
   func_L00_002D4040@003A5500, func_00273318@003A5544,
   func_00282948@003A5570, func_L00_002D3F30@003A55FC,
   func_L00_002AF568@003A5688, func_L00_002D6998@003A569C,
   func_002A7940@003A56AC, func_002A7470@003A56C4, func_00283CE0@003A56CC,
   func_00283CE0@003A56DC, func_002A7940@003A56E8, func_002A7940@003A56F0,
   func_00283CE0@003A5704, func_002E5CF8@003A573C,
   func_L00_002D4040@003A5764, func_L00_002D6B70@003A578C,
   func_00282978@003A57A8, func_00282978@003A57B0, func_00282948@003A57B8,
   func_L00_002E6EF8@003A57D0, func_00282978@003A57EC,
   func_L00_0032A1A8@003A5818, func_0029FCC0@003A5850,
   func_L00_003A5FC0@003A585C, func_L00_002E6EF8@003A587C,
   func_002E5CF8@003A588C, func_00282D30@003A595C, func_00282CC0@003A596C,
   func_00282D30@003A5988, func_00282CC0@003A5998, func_00277F48@003A59B4,
   func_00282D30@003A5A58, func_002A8E20@003A5A68, func_0032D758@003A5A78,
   func_00283410@003A5A90, func_00283678@003A5A9C, func_0029FCC0@003A5AAC,
   func_00282D30@003A5ADC, func_00282D30@003A5AEC, func_00282D30@003A5B04,
   func_00283738@003A5B14, func_00282D30@003A5B4C, func_00283738@003A5B5C,
   func_002A7878@003A5C00, func_002AB668@003A5C10, func_002ACDE8@003A5C20,
   func_002ACDE8@003A5C30, func_002A7878@003A5C4C, func_00283B98@003A5C58,
   func_002AB668@003A5C6C, func_002ACDE8@003A5C7C, func_002ACDE8@003A5C8C,
   func_00282D30@003A5CA0, func_00282CC0@003A5CB0, func_00282CC0@003A5CC0,
   func_L00_003A5DC0@003A5CD4]; data addresses materialised [0x190000,
   0x190000, 0x190000, 0x600f0000, 0x60000000, 0x600f0000, 0x3fa60000,
   0x190000, 0x190000, 0x140000, 0x140000, 0x260000, 0x290000, 0x190000,
   0x190000, 0x3a0000, 0x3a0000, 0x3d750000, 0x1c0000, 0x3df50000, 0x140000,
   0x260000, 0xbfac0000, 0x3e8c0000, 0x1c0000, 0x45af0000, 0x190000,
   0x3dcc0000, 0xbdcc0000, 0x3f860000, 0x3ea70000, 0x3c230000, 0x140000,
   0x260000]; float constants [0x42b40000, 0x3f800000, 0x190000, 0x3f800000,
   0x10000, 0x3fc00000, 0xbfc00000, 0xbf800000, 0x3f000000, 0x3fc00000,
   0x42000000, 0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7A020000@0x043c, LQ/SQ 7C620000@0x0440, LQ/SQ 78820000@0x0484, LQ/SQ
   7C620000@0x0488, LQ/SQ 7A020000@0x073c, LQ/SQ 7EA20000@0x0740, LQ/SQ
   78620000@0x074c, LQ/SQ 7C820000@0x0750, LQ/SQ 7AA20000@0x0780, LQ/SQ
   7C620000@0x0784]. Head: addiu $sp, $sp, -0x2C0 ; sd $s4, 0x280($sp) ; sd
   $s0, 0x260($sp) ; daddu $s4, $a0, $zero ; sd $s1, 0x268($sp) ; sd $s2,
   0x270($sp). Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
