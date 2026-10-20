/* func_L07_00410B48 - 260 bytes, 3 place(s), level 07_siberius, retail at
   0x00410b48. Screened with tools/w10_screen.py: 6 call(s)
   [func_002A7750@00410B9C, func_002A9640@00410BB4, func_002A7750@00410BD8,
   func_002E5CF8@00410BF0, func_L00_0032A258@00410C28,
   func_0029F338@00410C30]; data addresses materialised [0x4010000]; float
   constants [-]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [-]. Head: addiu $sp, $sp,
   -0x40 ; addiu $v0, $zero, 0x1 ; sd $s0, 0x20($sp) ; sd $s1, 0x28($sp) ;
   daddu $s0, $a0, $zero ; sd $ra, 0x30($sp). Not matched in this wave: the
   body is straight-line code whose register allocation and the
   materialisation of its constant addresses have to be reproduced word for
   word. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
