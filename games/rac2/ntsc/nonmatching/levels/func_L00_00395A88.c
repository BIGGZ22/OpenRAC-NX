/* func_L00_00395A88 - 1516 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00395a88. Screened with tools/w10_screen.py: 39 call(s)
   [func_0027C878@00395AEC, func_0027C878@00395B3C, func_00282CF0@00395BD8,
   func_00282CF0@00395BEC, func_00282DD8@00395C00, func_00282DF0@00395C0C,
   func_00282D30@00395C3C, func_00282CC0@00395C4C, func_00282CF0@00395C5C,
   func_00282CF0@00395C94, func_00282CF0@00395CA4, func_00282DD8@00395CB4,
   func_00282DF0@00395CBC, func_00282D30@00395CF0, func_00282CC0@00395D00,
   func_00282CF0@00395D10, func_00280B90@00395D70, func_00282CF0@00395DF8,
   func_00282CF0@00395E08, func_00282DF0@00395E10, func_00282DF0@00395E1C,
   func_00282D30@00395E60, func_00282D30@00395E9C, func_00282CC0@00395EB0,
   func_00282CC0@00395EC4, func_00282CF0@00395ED4, func_00282D30@00395EE4,
   func_00282CF0@00395EF4, func_00282CC0@00395F04, func_00282CF0@00395F14,
   func_00282DD8@00395F24, func_00282DF0@00395F2C, func_00282D30@00395F60,
   func_00282CC0@00395F70, func_00282CF0@00395F80, func_00282CC0@00395F90,
   func_00282CF0@00395FA0, func_00280B90@00395FF8, func_002FC560@00396028];
   data addresses materialised [0x1c0000, 0x1c0000, 0x1c0000, 0x50000];
   float constants [0x3f800000, 0x3f800000, 0x3f800000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 7A420000@0x02f0, LQ/SQ 7FA20000@0x02f4, LQ/SQ
   7A620000@0x02fc, LQ/SQ 7C620000@0x0300, LQ/SQ 7A620000@0x03bc, LQ/SQ
   7E220000@0x03c0, LQ/SQ 7A220000@0x03f8, LQ/SQ 7E620000@0x03fc]. Head:
   addiu $sp, $sp, -0x260 ; sd $s6, 0x210($sp) ; daddu $s6, $a0, $zero ; sd
   $s0, 0x1E0($sp) ; sd $s1, 0x1E8($sp) ; daddu $s0, $a1, $zero. Not matched
   in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches
   it only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
