/* func_L18_00427208 - 488 bytes, 3 place(s), level 18_damosel, retail at
   0x00427208. Screened with tools/w10_screen.py: 6 call(s)
   [func_L18_00426F28@004272B4, func_L18_00427130@004272DC,
   func_002829A8@00427308, func_002A7750@00427394,
   func_L02_002EF388@004273B4, func_002A7750@004273C4]; data addresses
   materialised [0x3dcc0000, 0x3f330000, 0x1b0000]; float constants
   [0x3f800000, 0x1b0000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   78620000@0x0048, LQ/SQ 7CA20000@0x004c, LQ/SQ 78820000@0x0058, LQ/SQ
   7C620000@0x005c, COP1 46800020@0x0084]. Head: addiu $sp, $sp, -0x40 ; sd
   $s0, 0x0($sp) ; sd $s4, 0x20($sp) ; daddu $s0, $a0, $zero ; sd $s2,
   0x10($sp) ; addiu $s4, $zero, -0x1. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
