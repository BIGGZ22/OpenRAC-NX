/* func_L11_00460258 - 420 bytes, 2 place(s), level 11_joba, retail at
   0x00460258. Screened with tools/w10_screen.py: 12 call(s)
   [func_00283CE0@0046028C, func_002A7878@004602C4, func_002A7878@00460300,
   func_00283180@0046030C, func_00283198@0046031C, func_002A7878@00460330,
   func_002A77E0@00460340, func_002A7820@00460350,
   func_L02_003449D8@00460368, func_002A7878@00460380,
   func_002C65A0@004603B0, func_00283B98@004603BC]; data addresses
   materialised [0x40c90000, 0xc0490000, 0x40490000, 0x3c880000,
   0x484d0000]; float constants [0x40a00000, 0x40c00000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [-]. Head: addiu $sp, $sp, -0x60 ; sd $s0, 0x10($sp) ; sd
   $s2, 0x20($sp) ; daddu $s0, $a1, $zero ; daddu $s2, $a0, $zero ; swc1
   $f24, 0x50($sp). Not matched in this wave: the body is straight-line code
   whose register allocation and the materialisation of its constant
   addresses have to be reproduced word for word. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
