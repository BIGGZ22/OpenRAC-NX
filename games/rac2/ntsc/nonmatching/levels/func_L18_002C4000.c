/* func_L18_002C4000 - 496 bytes, 3 place(s), level 18_damosel, retail at
   0x002c4000. Screened with tools/w10_screen.py: 6 call(s)
   [func_L00_002D2168@002C4084, func_L00_002D2168@002C40CC,
   func_L00_002D2168@002C40E8, func_L18_00428428@002C416C,
   func_L18_00428428@002C419C, func_L18_00428428@002C41D4]; data addresses
   materialised [0x190000, 0x140000, 0xbff0000]; float constants
   [0x40a00000, 0x40400000, 0x41000000, 0x41100000, 0x40400000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [-]. Head: addiu $sp, $sp, -0x20 ; lui $v0,
   0x19 ; sd $s0, 0x0($sp) ; addiu $v1, $zero, 0xEC0 ; sd $s1, 0x8($sp) ;
   addiu $s0, $v0, -0x61E0. Not matched in this wave: the body is straight-
   line code whose register allocation and the materialisation of its
   constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
