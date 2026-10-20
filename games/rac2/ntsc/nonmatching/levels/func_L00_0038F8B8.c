/* func_L00_0038F8B8 - 908 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038f8b8. Screened with tools/w10_screen.py: 14 call(s)
   [func_00282948@0038F8EC, func_00282948@0038F910,
   func_L00_002D3F30@0038FA70, func_00282E20@0038FAA0,
   func_00283248@0038FAAC, func_002AF000@0038FACC, func_00282CC0@0038FADC,
   func_002AF000@0038FAFC, func_00282CC0@0038FB0C, func_002A7470@0038FB2C,
   func_L00_002F46C8@0038FB34, func_L00_00390FF8@0038FB48,
   func_002E5CF8@0038FBDC, func_002A7470@0038FC08]; data addresses
   materialised [0x190000, 0x1b0000, 0x1b0000, 0x140000, 0x1c0000,
   0x3f190000, 0x3f190000, 0x190000, 0x1a0000]; float constants [0x1a0000,
   0x80000000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x50 ; sd $s0, 0x10($sp) ; sd $s1, 0x18($sp) ; daddu $s0, $a0, $zero ;
   sd $s2, 0x20($sp) ; sd $s3, 0x28($sp). Not matched in this wave: the body
   is straight-line code whose register allocation and the materialisation
   of its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
