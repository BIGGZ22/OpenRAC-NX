/* func_L11_004507B0 - 632 bytes, 2 place(s), level 11_joba, retail at
   0x004507b0. Screened with tools/w10_screen.py: 16 call(s)
   [func_L11_004506C0@00450808, func_0027C878@00450810,
   func_00283248@00450884, func_00282E80@00450894, func_00283248@004508A8,
   func_00283410@004508C4, func_00282CF0@004508D8, func_00282F20@004508E8,
   func_L11_00450748@00450930, func_00280B90@00450940,
   func_00282F20@00450960, func_00282CC0@00450980, func_00281EB8@004509A4,
   func_00282F20@004509BC, func_00282CC0@004509CC, func_00281EB8@004509EC];
   data addresses materialised [0x1c0000, 0x3da30000, 0xbcf50000]; float
   constants [0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [-]. Head: addiu
   $sp, $sp, -0x160 ; sd $s0, 0x100($sp) ; sd $s1, 0x108($sp) ; daddu $s0,
   $t0, $zero ; sd $s2, 0x110($sp) ; daddu $s1, $a1, $zero. Not matched in
   this wave: the body is straight-line code whose register allocation and
   the materialisation of its constant addresses have to be reproduced word
   for word. What would unblock it: the exact source expression tree, so
   this cc1 picks the same registers in the same order. */
