/* func_L08_003C6528 - 456 bytes, 3 place(s), level 08_tabora, retail at
   0x003c6528. Screened with tools/w10_screen.py: 4 call(s)
   [func_002A7798@003C658C, func_002A7798@003C6598, func_002A7798@003C65EC,
   func_002A7798@003C65F8]; data addresses materialised [0x190000, 0x140000,
   0x1a0000]; float constants [0x3f800000, 0xbf800000, 0x3f800000,
   0xbf800000, 0x3f800000, 0xbf800000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [-]. Head:
   addiu $sp, $sp, -0x50 ; lui $v0, 0x19 ; sd $s0, 0x0($sp) ; addiu $v0,
   $v0, -0x61E0 ; sd $s1, 0x8($sp) ; lui $s0, 0x14. Not matched in this
   wave: the body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
