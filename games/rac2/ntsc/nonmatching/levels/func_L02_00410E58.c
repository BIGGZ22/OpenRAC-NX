/* func_L02_00410E58 - 272 bytes, 7 place(s), level 02_maktar_nebula, retail
   at 0x00410e58. Screened with tools/w10_screen.py: 2 call(s)
   [func_0029EFD0@00410EB8, func_00283B98@00410F1C]; data addresses
   materialised [0x190000, 0x40060000]; float constants [-]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 7A820000@0x009c, LQ/SQ 7C620000@0x00a0, LQ/SQ
   7A620000@0x00a8, LQ/SQ 7C820000@0x00ac, COP1 46806B60@0x00b8]. Head:
   addiu $sp, $sp, -0x60 ; lui $v0, 0x19 ; sd $s0, 0x0($sp) ; sd $s1,
   0x8($sp) ; sd $s2, 0x10($sp) ; daddu $s1, $zero, $zero. Not matched in
   this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it
   only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
