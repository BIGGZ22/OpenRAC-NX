/* func_L00_003914E0 - 1000 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003914e0. Screened with tools/w10_screen.py: 17 call(s)
   [func_002BC768@00391584, func_002BC768@003915D8, func_00282CF0@00391600,
   func_00282C88@0039160C, func_00282DF0@00391614, func_00283CF0@0039163C,
   func_002A7878@00391690, func_002A77E0@00391714, func_002A77E0@00391724,
   func_002A7820@00391738, func_002A7820@00391748, func_002BBCB0@0039177C,
   func_00308408@003917D4, func_00308408@00391800, func_00308408@0039182C,
   func_00308408@00391858, func_00308408@00391884]; data addresses
   materialised [0x40800000, 0x3e990000, 0x3f810000, 0x3f890000, 0x3d4c0000,
   0x47430000, 0x40800000, 0x3e990000, 0x3f810000, 0x3f880000, 0x3d4c0000,
   0x47430000, 0x40800000, 0x1c0000, 0x3c880000, 0x1b0000, 0x1b0000,
   0x48c30000, 0x1b0000, 0x3f730000]; float constants [0x41000000,
   0x40e00000, 0x41000000, 0x41200000, 0x40400000, 0x40000000, 0x40400000,
   0x40000000, 0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   78620000@0x0044, LQ/SQ 7FA20000@0x0048]. Head: addiu $sp, $sp, -0xE0 ; sd
   $s5, 0x98($sp) ; sd $s1, 0x78($sp) ; daddu $s5, $a0, $zero ; sd $s0,
   0x70($sp) ; daddu $s1, $a1, $zero. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
