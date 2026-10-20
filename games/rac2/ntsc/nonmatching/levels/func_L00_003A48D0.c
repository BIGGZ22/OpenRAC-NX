/* func_L00_003A48D0 - 2708 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a48d0. Screened with tools/w10_screen.py: 56 call(s)
   [func_L00_003A41F0@003A4948, func_002829D8@003A4950,
   func_L00_002C42E0@003A4974, func_002A75E0@003A49A8,
   func_L00_002D4040@003A4A28, func_00273318@003A4A68,
   func_00282D30@003A4ACC, func_00282D30@003A4B04, func_00282CC0@003A4B18,
   func_L00_00324D18@003A4B40, func_002AB1F0@003A4B54,
   func_00282948@003A4BD4, func_00282E48@003A4BFC, func_002AB1F0@003A4C14,
   func_00282D30@003A4C40, func_00282CC0@003A4C50, func_00282C48@003A4C74,
   func_00273318@003A4CA4, func_00282948@003A4CC8, func_002AB1F0@003A4CE0,
   func_00282948@003A4D30, func_002A7470@003A4DE0,
   func_L00_002D3F30@003A4DEC, func_00282D30@003A4E14,
   func_00282CC0@003A4E24, func_00282D30@003A4E40, func_00282CC0@003A4E50,
   func_00277F48@003A4E6C, func_002AB1F0@003A4EA4, func_002ABB78@003A4F14,
   func_002E5CF8@003A4F64, func_L00_003A4480@003A4F74,
   func_L00_003A5370@003A4F7C, func_L00_002D2168@003A4FD4,
   func_L00_002AF568@003A4FE4, func_002A7470@003A4FF8,
   func_L00_002D6998@003A5004, func_002E5CF8@003A507C,
   func_L00_002D4040@003A50B0, func_L00_002D6B70@003A50D8,
   func_00283CE0@003A510C, func_00283CE0@003A512C, func_00283CE0@003A513C,
   func_00283CE0@003A5160, func_00283CE0@003A517C, func_00283CE0@003A5188,
   func_00283CE0@003A51A4, func_00282D30@003A51E0, func_0029FCC0@003A520C,
   func_00282CC0@003A5220, func_00282D30@003A5248, func_00282CC0@003A5258,
   func_L00_002FCF18@003A5288, func_00282978@003A52C8,
   func_00291450@003A52E8, func_00291450@003A5318]; data addresses
   materialised [0x190000, 0x190000, 0x190000, 0x3d8e0000, 0x60000000,
   0x600f0000, 0x190000, 0x3ea00000, 0x3e970000, 0x190000, 0x190000,
   0x190000, 0x190000, 0x1c0000, 0x3bda0000, 0x3dcc0000, 0x3f260000,
   0x3f660000, 0x3dcc0000, 0x190000, 0x1b0000, 0x190000, 0x140000,
   0x3e8c0000, 0x1c0000, 0x1c0000, 0x190000, 0x3f330000, 0x290000, 0x140000,
   0x1b0000, 0x140000, 0x1b0000, 0x140000, 0x190000, 0x3f4c0000, 0x3f4c0000,
   0x190000, 0x190000, 0x3e990000, 0x1c0000]; float constants [0x3fa00000,
   0x42b40000, 0x3f400000, 0x190000, 0xbf800000, 0x3f400000, 0x42960000,
   0xff0f0000, 0x42b40000, 0x190000, 0xbf800000, 0x3f800000, 0x1c0000,
   0x41200000, 0x260000, 0x3f800000, 0x3f800000, 0x3f800000, 0x3f000000,
   0xbf800000, 0x41000000, 0x41000000]; $gp-relative small data: yes
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 7A020000@0x01a8, LQ/SQ 7FA20000@0x01ac, LQ/SQ 78620000@0x0210,
   LQ/SQ 7FA20000@0x0214, LQ/SQ 78620000@0x0928, LQ/SQ 7E220000@0x092c,
   LQ/SQ 7A620000@0x09d8, LQ/SQ 7C820000@0x09dc]. Head: addiu $sp, $sp,
   -0xD0 ; sd $s4, 0x80($sp) ; sd $s0, 0x60($sp) ; daddu $s4, $a0, $zero ;
   sd $s1, 0x68($sp) ; sd $s2, 0x70($sp). Not matched in this wave: the
   128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
