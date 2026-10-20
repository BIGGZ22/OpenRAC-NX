/* func_L18_002D1998 - 836 bytes, 2 place(s), level 18_damosel, retail at
   0x002d1998. Screened with tools/w10_screen.py: 20 call(s)
   [func_L00_002D3A58@002D19D0, func_L00_002D36E0@002D1A68,
   func_L00_002D36E0@002D1A90, func_00282E20@002D1AA4,
   func_00282F20@002D1ADC, func_00282F20@002D1B04,
   func_L00_002D3B58@002D1B20, func_00282F20@002D1B34,
   func_00282D30@002D1B54, func_00282CC0@002D1B68, func_00282CC0@002D1B78,
   func_002761F8@002D1B94, func_00278E78@002D1BB0, func_002761F8@002D1BD4,
   func_L00_002D3EA8@002D1BF4, func_L00_002D3B58@002D1C5C,
   func_00282F20@002D1C70, func_00282DB0@002D1C7C, func_00282D30@002D1C98,
   func_00282CC0@002D1CA8]; data addresses materialised [0x190000,
   0x3ada0000, 0x3ebd0000, 0x3f330000, 0x3eb30000, 0x3e990000, 0x190000,
   0x3c230000, 0x190000, 0x3fb30000, 0x190000, 0x3fb30000, 0x1c0000,
   0x1c0000, 0x3f5f0000, 0x1c0000, 0x1c0000]; float constants [0x3f000000,
   0x3f800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x90 ; lui $v0, 0x19 ; sd $s0, 0x40($sp) ; sd $s1, 0x48($sp) ; addiu
   $s0, $v0, -0x60F0 ; sd $s2, 0x50($sp). Not matched in this wave: the body
   is straight-line code whose register allocation and the materialisation
   of its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
