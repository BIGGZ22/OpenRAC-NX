/* func_L05_0037AC80 - 340 bytes, 4 place(s), level 05_feltzin_system,
   retail at 0x0037ac80. Screened with tools/w10_screen.py: 9 call(s)
   [func_L05_00445980@0037AC98, func_00283CF0@0037ACBC,
   func_002ED8A0@0037ACE0, func_00283CF0@0037AD0C, func_00283CE0@0037AD28,
   func_00283CF0@0037AD4C, func_00283CF0@0037AD60,
   func_L05_00442398@0037ADA0, func_L00_002B30E8@0037ADB0]; data addresses
   materialised [0x220000, 0x1c0000]; float constants [0x3f000000,
   0x2a0000]; $gp-relative small data: no   saves: -; callee-saved registers
   saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp, -0x30 ;
   swc1 $f20, 0x20($sp) ; sd $s0, 0x0($sp) ; sd $s1, 0x8($sp) ; sd $s2,
   0x10($sp) ; sd $ra, 0x18($sp). Not matched in this wave: the body is
   straight-line code whose register allocation and the materialisation of
   its constant addresses have to be reproduced word for word. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
