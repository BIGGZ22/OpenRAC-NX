/* func_L00_003A2EE0 - 3188 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a2ee0. Screened with tools/w10_screen.py: 54 call(s)
   [func_0026FF08@003A2F88, func_00291450@003A2F9C,
   func_L00_00368F58@003A2FB4, func_00291450@003A2FF4,
   func_0026FF08@003A305C, func_L00_00368F58@003A3064,
   func_002A75E0@003A30BC, func_00282F20@003A310C, func_00282CC0@003A311C,
   func_002761F8@003A3134, func_002AB1F0@003A315C, func_002ABB78@003A3168,
   func_L00_002D4040@003A321C, func_00273318@003A3268,
   func_L00_002D4040@003A328C, func_00273318@003A32CC,
   func_00283248@003A3338, func_L00_00329E90@003A33C4,
   func_L00_0032A088@003A33E0, func_L00_0032A0B8@003A33F0,
   func_L00_00329FF0@003A3400, func_L00_003A8508@003A341C,
   func_L00_002C42E0@003A3440, func_00282948@003A3458,
   func_002E5CF8@003A350C, func_002E5CF8@003A35A0, func_002E59F8@003A360C,
   func_0026FF08@003A367C, func_L00_00368F58@003A3684,
   func_002E59F8@003A36CC, func_0026FF08@003A3734,
   func_L00_00368F58@003A374C, func_L00_002D3F30@003A3810,
   func_L00_002AF568@003A3820, func_L00_002D6998@003A382C,
   func_002A7470@003A3850, func_002E5CF8@003A3864,
   func_L00_002D4040@003A3888, func_L00_002D6B70@003A38B0,
   func_00282978@003A38D4, func_002E59F8@003A3928, func_002E5CF8@003A3940,
   func_L00_003A3B58@003A3948, func_00288E18@003A39A4,
   func_002829D8@003A39B4, func_00282948@003A39D0, func_00291450@003A39EC,
   func_00291308@003A3A20, func_0029FCC0@003A3A50, func_0029FCC0@003A3A74,
   func_00282F20@003A3A8C, func_00282CC0@003A3AAC, func_00283CE0@003A3AD0,
   func_00283CE0@003A3ADC]; data addresses materialised [0x190000,
   0x3f8f0000, 0x3ba30000, 0x3e4c0000, 0x1b0000, 0x1b0000, 0x1b0000,
   0x3f8f0000, 0x3ba30000, 0x3e4c0000, 0x1b0000, 0x1b0000, 0x1b0000,
   0x1c0000, 0x1c0000, 0x40000000, 0x600f0000, 0x60000000, 0x600f0000,
   0x3ebb0000, 0x1c0000, 0x3e320000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000,
   0x1b0000, 0x140000, 0x3e320000, 0x190000, 0x190000, 0x3c230000, 0x190000,
   0x190000, 0x3c230000, 0x190000, 0x3e320000, 0x3ca30000, 0x3e4c0000,
   0x190000, 0x140000, 0x3e320000, 0x3ba30000, 0x3e4c0000, 0x1b0000,
   0x3e320000, 0x3cf50000, 0x3ecc0000, 0x3c2b0000, 0x1b0000, 0x1b0000,
   0x290000, 0x190000, 0x140000, 0x260000, 0x1c0000]; float constants
   [0x43000000, 0x3fc00000, 0x42b40000, 0x40000000, 0x42b40000, 0x3f800000,
   0x3e800000, 0x190000, 0x190000, 0x3e800000, 0x3f800000, 0x41700000,
   0x7f600000, 0xbfc00000, 0x3fc00000, 0x41700000, 0x3f400000]; $gp-relative
   small data: yes   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 78620000@0x01a8, LQ/SQ 7FA20000@0x01ac]. Head: addiu
   $sp, $sp, -0x90 ; lui $v0, 0x19 ; sd $s3, 0x58($sp) ; addiu $v1, $zero,
   0x3 ; sd $s5, 0x68($sp) ; daddu $s3, $a0, $zero. Not matched in this
   wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only
   through a `mode(TI)` local, and this cc1 folds the destination offset
   into the `sq` where retail materialises the pointer first. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
