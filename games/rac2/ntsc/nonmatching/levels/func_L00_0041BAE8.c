/* func_L00_0041BAE8 - 476 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041bae8. Screened with tools/w10_screen.py: 4 call(s)
   [func_00300190@0041BB74, func_002AA3A8@0041BBEC, func_002AA3A8@0041BC48,
   func_00282948@0041BC6C]; data addresses materialised [0x3b030000,
   0x39510000, 0x3ca30000, 0x3b030000, 0x39510000, 0x3ca30000]; float
   constants [0x3f000000, 0xc1000000, 0x41000000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [-]. Head: addiu $sp, $sp, -0x60 ; daddu $a2, $zero, $zero ; swc1
   $f23, 0x58($sp) ; swc1 $f22, 0x50($sp) ; sd $s6, 0x30($sp) ; sd $s0,
   0x0($sp). Not matched in this wave: the body is straight-line code whose
   register allocation and the materialisation of its constant addresses
   have to be reproduced word for word. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
