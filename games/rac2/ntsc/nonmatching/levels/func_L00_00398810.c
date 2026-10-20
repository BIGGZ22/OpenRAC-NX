/* func_L00_00398810 - 732 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00398810. Screened with tools/w10_screen.py: 6 call(s)
   [func_0029EFD0@00398834, func_002FED90@003988D8, func_002AB650@003988E4,
   func_002ACAD0@00398A88, func_002ACA50@00398AB8, func_002A0F48@00398AC0];
   data addresses materialised [0x190000, 0x140000, 0x260000, 0x1b0000,
   0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000]; float constants
   [0x3f800000, 0x42700000, 0x40c00000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 7A420000@0x0058, LQ/SQ 7C820000@0x005c, LQ/SQ 7A620000@0x0064,
   LQ/SQ 7C620000@0x0068, LQ/SQ 78620000@0x0080, LQ/SQ 7CC20000@0x0084,
   LQ/SQ 78A20000@0x0090, LQ/SQ 7C620000@0x0094, LQ/SQ 78820000@0x00a0,
   LQ/SQ 7C620000@0x00a4]. Head: addiu $sp, $sp, -0x30 ; sd $s2, 0x10($sp) ;
   sd $s3, 0x18($sp) ; daddu $s2, $a0, $zero ; sd $s1, 0x8($sp) ; daddu $s3,
   $a1, $zero. Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
