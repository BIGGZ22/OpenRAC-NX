/* func_L16_00395C00 - 304 bytes, 2 place(s), level 16_snivelak, retail at
   0x00395c00. Screened with tools/w10_screen.py: 8 call(s)
   [func_002AFDD8@00395C30, func_00283248@00395C3C, func_00282C48@00395C8C,
   func_00283198@00395CC8, func_00282F20@00395CD8, func_00283180@00395CE4,
   func_00283958@00395CF8, func_00283738@00395D08]; data addresses
   materialised [0x190000, 0x3e560000, 0xbe560000, 0x3ae40000]; float
   constants [0x3f000000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [-]. Head: addiu
   $sp, $sp, -0x90 ; lui $a2, 0x19 ; sd $s3, 0x68($sp) ; addiu $a2, $a2,
   -0x61E0 ; sd $s0, 0x50($sp) ; daddu $s3, $a0, $zero. Not matched in this
   wave: the body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
