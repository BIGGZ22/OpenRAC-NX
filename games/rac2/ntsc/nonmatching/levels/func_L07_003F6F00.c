/* func_L07_003F6F00 - 324 bytes, 4 place(s), level 07_siberius, retail at
   0x003f6f00. Screened with tools/w10_screen.py: 10 call(s)
   [func_002A7820@003F6F30, func_002A77E0@003F6F3C, func_002C3150@003F6F5C,
   func_00282C88@003F6F64, func_002A7A00@003F6F98, func_00282CC0@003F6FA8,
   func_002A7878@003F6FB8, func_002A7820@003F6FD8, func_002A77E0@003F6FE4,
   func_002C3150@003F700C]; data addresses materialised [0x1b0000,
   0x3c880000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000]; float constants
   [0x3f000000, 0x1b0000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [-]. Head: addiu
   $sp, $sp, -0x60 ; lui $a1, 0x1B ; lw $a1, 0x1004($a1) ; sd $s3, 0x38($sp)
   ; daddu $s3, $a0, $zero ; sd $s0, 0x20($sp). Not matched in this wave:
   the body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
