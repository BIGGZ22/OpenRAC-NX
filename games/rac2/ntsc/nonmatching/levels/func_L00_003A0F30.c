/* func_L00_003A0F30 - 1616 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a0f30. Screened with tools/w10_screen.py: 36 call(s)
   [func_00282948@003A0F80, func_002A7470@003A1018, func_0029F978@003A103C,
   func_L00_00329E90@003A1060, func_L00_0032A088@003A107C,
   func_002A78D8@003A10AC, func_002A78D8@003A10C0, func_002A78D8@003A10D0,
   func_002A7878@003A10E0, func_002A7878@003A10F0, func_002A7878@003A1100,
   func_002A78D8@003A111C, func_002A78D8@003A112C, func_002A78D8@003A113C,
   func_002A7878@003A114C, func_002A7878@003A115C, func_002A7878@003A116C,
   func_00282C88@003A1180, func_00282C88@003A1188,
   func_L00_002C42E0@003A11D4, func_L00_002D4040@003A1208,
   func_00273318@003A1248, func_002829D8@003A1250,
   func_L00_003BB5F0@003A1290, func_L00_002D3F30@003A12B4,
   func_002E5CF8@003A12D4, func_002E5CF8@003A1390, func_0029FCC0@003A1434,
   func_00282F20@003A1460, func_00282F20@003A1470,
   func_L00_0032A1A8@003A1494, func_L00_003BB080@003A14A8,
   func_L00_002E6EF8@003A1504, func_00283B98@003A1528,
   func_00283198@003A1534, func_00283180@003A1544]; data addresses
   materialised [0x190000, 0x3ebb0000, 0x1b0000, 0x3d4c0000, 0x3e990000,
   0x240000, 0x1b0000, 0x600f0000, 0x60000000, 0x600f0000, 0x140000,
   0x1b0000, 0x140000, 0x140000, 0x260000, 0x260000, 0x1b0000, 0x1b0000,
   0x3eaa0000, 0x3eaa0000, 0x3a0000, 0x3ea00000]; float constants
   [0xbf800000, 0x3e800000, 0x3f400000, 0xbf800000, 0x3f800000, 0x42b40000,
   0x3f800000, 0x3f800000, 0x3f000000]; $gp-relative small data: yes
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [-]. Head: addiu $sp, $sp, -0x80 ; lui $v0, 0x19 ; sd $s2, 0x40($sp) ; sd
   $s3, 0x48($sp) ; daddu $s2, $a0, $zero ; sd $s4, 0x50($sp). Not matched
   in this wave: the body is straight-line code whose register allocation
   and the materialisation of its constant addresses have to be reproduced
   word for word. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
