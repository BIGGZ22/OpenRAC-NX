/* func_L00_0039D110 - 560 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0039d110. Screened with tools/w10_screen.py: 12 call(s)
   [func_0029FCC0@0039D148, func_002FDF40@0039D1A8, func_002FE5A0@0039D1D0,
   func_00282948@0039D1E8, func_L00_002D3F30@0039D288,
   func_L00_002AF568@0039D298, func_002E5CF8@0039D2B0,
   func_00282948@0039D2C0, func_L00_00398810@0039D2D0,
   func_L00_002AF7D0@0039D2D8, func_00282948@0039D2F4,
   func_L00_002E6EF8@0039D30C]; data addresses materialised [0x190000,
   0x3b480000, 0x3e2a0000, 0x3a0000, 0x3a0000, 0x140000, 0x3a0000, 0x3a0000,
   0x3a0000, 0x3a0000]; float constants [0x40a00000, 0x41300000,
   0x3fc00000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x70 ; daddu $a1, $zero, $zero ; sd $s0, 0x20($sp) ; daddu $a2, $sp,
   $zero ; sd $s2, 0x30($sp) ; daddu $s0, $a0, $zero. Not matched in this
   wave: the body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
