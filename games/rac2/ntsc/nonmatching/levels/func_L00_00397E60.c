/* func_L00_00397E60 - 696 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00397e60. Screened with tools/w10_screen.py: 13 call(s)
   [func_0029FCC0@00397E98, func_002AF580@00397F14, func_002FDF40@00397F6C,
   func_002FE5A0@00397F90, func_00282948@00397FA8,
   func_L00_002D3F30@00398050, func_L00_002AF568@00398060,
   func_002E5CF8@00398088, func_00282948@00398098,
   func_L00_0038D530@003980A8, func_L00_002AF7D0@003980B0,
   func_00282948@003980CC, func_L00_002E6EF8@003980E4]; data addresses
   materialised [0x1b0000, 0x3f490000, 0x3ec90000, 0x41310000, 0x3b480000,
   0x3e2a0000, 0x3a0000, 0x3a0000, 0x140000, 0x190000, 0x3a0000, 0x3a0000,
   0x3a0000]; float constants [0x190000, 0x41200000, 0x41300000,
   0x3fc00000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x90 ; daddu $a1, $zero, $zero ; sd $s0, 0x40($sp) ; daddu $a2, $sp,
   $zero ; sd $s1, 0x48($sp) ; daddu $s0, $a0, $zero. Not matched in this
   wave: the body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
