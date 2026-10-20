/* func_L08_003CD488 - 376 bytes, 3 place(s), level 08_tabora, retail at
   0x003cd488. Screened with tools/w10_screen.py: 11 call(s)
   [func_L00_00424A88@003CD4CC, func_L00_00424A08@003CD4DC,
   func_002ACBC8@003CD4F0, func_L00_00424B20@003CD50C,
   func_L00_00424A50@003CD520, func_L00_00424C28@003CD528,
   func_00283CE0@003CD530, func_00283CE0@003CD53C,
   func_L00_00424B70@003CD5B4, func_L00_00424BB8@003CD5C0,
   func_L00_00424C00@003CD5C8]; data addresses materialised [0x1c0000,
   0x190000, 0x3f060000, 0x3b440000, 0x3f7c0000, 0x3f4c0000, 0x3d380000,
   0x3c750000, 0x3e4c0000]; float constants [0x3f800000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [-]. Head: addiu $sp, $sp, -0x30 ; lui $v0, 0x1C ; sd $s1,
   0x8($sp) ; sd $s0, 0x0($sp) ; daddu $s1, $a0, $zero ; sd $ra, 0x10($sp).
   Not matched in this wave: the body is straight-line code whose register
   allocation and the materialisation of its constant addresses have to be
   reproduced word for word. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
