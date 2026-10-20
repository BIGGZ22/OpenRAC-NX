/* func_L08_00339220 - 568 bytes, 2 place(s), level 08_tabora, retail at
   0x00339220. Screened with tools/w10_screen.py: 3 call(s)
   [func_00282CF0@003392F4, func_00282DF0@00339304, func_0026FEB8@00339410];
   data addresses materialised [0x190000, 0xfffe0000, 0xc4790000, 0x1b0000];
   float constants [-]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7BA20010@0x00dc,
   LQ/SQ 7FA20000@0x00e8, COP1 46800020@0x0100, LQ/SQ 7A620010@0x0104, COP1
   468010A0@0x0114, LQ/SQ 7E020000@0x0118, COP1 46000064@0x0130]. Head:
   addiu $sp, $sp, -0x80 ; sd $s0, 0x20($sp) ; sd $s2, 0x30($sp) ; daddu
   $s0, $a1, $zero ; sd $s3, 0x38($sp) ; daddu $s2, $a2, $zero. Not matched
   in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches
   it only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
