/* func_L18_00426AB0 - 740 bytes, 3 place(s), level 18_damosel, retail at
   0x00426ab0. Screened with tools/w10_screen.py: 14 call(s)
   [func_00336D50@00426AFC, func_00300A00@00426B54, func_00336D50@00426B68,
   func_00300A00@00426BBC, func_00283CE0@00426BCC, func_00283CE0@00426BD8,
   func_00283D38@00426BEC, func_00283D38@00426C00, func_0027EAE8@00426C60,
   func_0027EAE8@00426CA8, 0x115da8@00426CF8, func_L18_003044B8@00426D24,
   func_L18_003044B8@00426D50, func_L18_003044B8@00426D6C]; data addresses
   materialised [0x190000, 0x1a0000, 0x60440000, 0x1a0000, 0x55f00000,
   0x1a0000, 0xff0000, 0xff0000, 0x1b0000, 0x3f750000, 0x3f850000]; float
   constants [0x1b0000, 0x3f800000, 0x1b0000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [COP1 46806320@0x008c, COP1 46806B60@0x009c, COP1 46806B60@0x00f4, COP1
   46806320@0x0104, COP1 46800860@0x0164, COP1 46800020@0x0198, COP1
   46000064@0x01a8, COP1 46800860@0x01c0, COP1 46800020@0x01e4, COP1
   46000064@0x01f0]. Head: addiu $sp, $sp, -0x50 ; lui $v0, 0x19 ; sd $s1,
   0x28($sp) ; addiu $a0, $zero, 0x4 ; sd $s0, 0x20($sp) ; addiu $s1, $v0,
   -0x61E0. Not matched in this wave: the float code has to come out of the
   same expression tree to keep the FPU registers. What would unblock it:
   the exact source expression tree, so this cc1 picks the same registers in
   the same order. */
