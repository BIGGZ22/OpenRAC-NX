/* func_L10_004191E8 - 352 bytes, 2 place(s), level 10_hrugis_cloud, retail
   at 0x004191e8. Screened with tools/w10_screen.py: 6 call(s)
   [func_002ED688@004191FC, func_002A77E0@00419278,
   func_L05_00379FA0@004192BC, func_L05_00379FA0@00419304,
   func_002A77E0@0041930C, func_002E5CF8@00419324]; data addresses
   materialised [0x60000000, 0x3f660000, 0x60800000, 0x3f190000,
   0x3f190000]; float constants [0x41500000, 0x40900000, 0x3f800000,
   0x3f800000, 0x40a00000, 0x3f800000, 0x40a00000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [-]. Head: addiu $sp, $sp, -0x20 ; sd $s0, 0x0($sp) ; sd $ra,
   0x10($sp) ; daddu $s0, $a0, $zero ; sd $s1, 0x8($sp) ; jal func_37BA88.
   Not matched in this wave: the body is straight-line code whose register
   allocation and the materialisation of its constant addresses have to be
   reproduced word for word. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
