/* func_L00_0038BD00 - 368 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038bd00. Screened with tools/w10_screen.py: 6 call(s)
   [func_00282E48@0038BDB0, func_002A7878@0038BDD8, func_003022A0@0038BDEC,
   func_L00_0031F2B0@0038BDFC, func_00282E48@0038BE10,
   func_L00_003D1B50@0038BE28]; data addresses materialised [0x1a0000,
   0x402c0000, 0x1b0000, 0x190000, 0x3c8e0000]; float constants [0x41200000,
   0xc1f00000, 0x41f00000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [-]. Head: addiu
   $sp, $sp, -0x50 ; lui $v0, 0x1A ; lbu $v0, 0x7B2A($v0) ; sd $s1, 0x8($sp)
   ; swc1 $f20, 0x40($sp) ; daddu $s1, $a0, $zero. Not matched in this wave:
   the body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
