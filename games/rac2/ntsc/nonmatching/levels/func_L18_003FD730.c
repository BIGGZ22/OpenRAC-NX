/* func_L18_003FD730 - 292 bytes, 3 place(s), level 18_damosel, retail at
   0x003fd730. Screened with tools/w10_screen.py: 5 call(s)
   [func_002820A0@003FD744, func_002AA140@003FD7AC, func_00283CF0@003FD7EC,
   func_00283CF0@003FD810, func_002820D0@003FD834]; data addresses
   materialised [0x1b0000, 0x1b0000, 0x3dcc0000, 0x3da30000]; float
   constants [0x3f800000, 0x41200000, 0x3f800000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [COP1 46806320@0x00ac, COP1 46800020@0x00d8]. Head: addiu $sp, $sp,
   -0x20 ; sd $s0, 0x0($sp) ; swc1 $f20, 0x10($sp) ; daddu $s0, $a0, $zero ;
   sd $ra, 0x8($sp) ; jal func_307860. Not matched in this wave: the float
   code has to come out of the same expression tree to keep the FPU
   registers. What would unblock it: the exact source expression tree, so
   this cc1 picks the same registers in the same order. */
