/* func_L00_0032D208 - 260 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0032d208. Screened with tools/w10_screen.py: 7 call(s)
   [0x132858@0032D244, 0x133230@0032D258, 0x132028@0032D260,
   0x132828@0032D27C, 0x132028@0032D288, 0x132318@0032D2DC,
   0x132828@0032D2E8]; data addresses materialised [0x190000, 0x150000,
   0xffff0000]; float constants [-]; $gp-relative small data: no   saves: -;
   callee-saved registers saved [-]; words plain C cannot emit [-]. Head:
   addiu $sp, $sp, -0x30 ; sd $s3, 0x18($sp) ; lui $s3, 0x19 ; sd $s0,
   0x0($sp) ; sd $s1, 0x8($sp) ; addiu $v0, $s3, -0x79A0. Not matched in
   this wave: the body is straight-line code whose register allocation and
   the materialisation of its constant addresses have to be reproduced word
   for word. What would unblock it: the exact source expression tree, so
   this cc1 picks the same registers in the same order. */
