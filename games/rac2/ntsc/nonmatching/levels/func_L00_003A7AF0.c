/* func_L00_003A7AF0 - 320 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a7af0. Screened with tools/w10_screen.py: 3 call(s)
   [func_L00_00329E90@003A7B74, func_L00_0032A088@003A7B94,
   func_L00_00329FF0@003A7BA0]; data addresses materialised [0x3dcc0000,
   0xff000000, 0x3dd60000, 0x1b0000, 0x1b0000, 0x3dd60000, 0x1b0000]; float
   constants [0x41f00000, 0x41a00000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [-]. Head:
   addiu $sp, $sp, -0x30 ; lui $at, 0x3DCC ; ori $at, $at, 0xCCCD ; mtc1
   $at, $f0 ; sd $s1, 0x18($sp) ; addiu $v1, $zero, -0x1. Not matched in
   this wave: the body is straight-line code whose register allocation and
   the materialisation of its constant addresses have to be reproduced word
   for word. What would unblock it: the exact source expression tree, so
   this cc1 picks the same registers in the same order. */
