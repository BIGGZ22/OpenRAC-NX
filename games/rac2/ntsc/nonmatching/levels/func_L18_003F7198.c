/* func_L18_003F7198 - 316 bytes, 3 place(s), level 18_damosel, retail at
   0x003f7198. Screened with tools/w10_screen.py: 4 call(s)
   [func_00282E48@003F71DC, func_002A93B0@003F7200,
   func_L18_003F74B8@003F7298, func_002B0990@003F72B8]; data addresses
   materialised [0x1c0000, 0x447e0000, 0x3fe60000]; float constants
   [0x41c00000, 0x40a00000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [-]. Head: addiu
   $sp, $sp, -0x20 ; ori $v0, $zero, 0xFFFE ; sd $s0, 0x0($sp) ; sd $s1,
   0x8($sp) ; daddu $s0, $a0, $zero ; sd $ra, 0x10($sp). Not matched in this
   wave: the body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
