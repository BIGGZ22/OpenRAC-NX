/* func_L00_002EC608 - 868 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x002ec608. Screened with tools/w10_screen.py: 27 call(s)
   [func_00281DE8@002EC698, func_002A7820@002EC6AC, func_00283248@002EC730,
   func_00282E80@002EC740, func_00283248@002EC754, func_002A7940@002EC760,
   func_00283410@002EC774, func_00282CF0@002EC788, func_00282F20@002EC79C,
   func_002A7878@002EC7EC, func_00282D30@002EC800, func_00282D30@002EC814,
   func_00282D30@002EC828, func_00282D30@002EC83C, func_00280B90@002EC84C,
   func_00282D30@002EC85C, func_00282D30@002EC86C, func_00282D30@002EC87C,
   func_00282D30@002EC88C, func_002A7820@002EC898, func_00280B90@002EC8C0,
   func_00282D30@002EC8D0, func_00282D30@002EC8E0, func_00282D30@002EC8F0,
   func_00282D30@002EC900, func_002A7820@002EC90C, func_00280B90@002EC93C];
   data addresses materialised [0x1c0000, 0x3ecc0000, 0x3e990000, 0x7f0000];
   float constants [0xc0000000, 0x3f800000, 0x40000000, 0x3f000000,
   0x3e800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x130 ; lui $at, 0xC000 ; mtc1 $at, $f1 ; swc1 $f20, 0x120($sp) ; addiu
   $a3, $zero, 0x80 ; sd $s1, 0xF8($sp). Not matched in this wave: the body
   is straight-line code whose register allocation and the materialisation
   of its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
