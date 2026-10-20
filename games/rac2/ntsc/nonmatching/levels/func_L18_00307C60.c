/* func_L18_00307C60 - 932 bytes, 2 place(s), level 18_damosel, retail at
   0x00307c60. Screened with tools/w10_screen.py: 13 call(s)
   [func_00282DD8@00307CD8, func_00282F20@00307CF0, func_00282DD8@00307D00,
   func_00282D30@00307D14, func_00282CC0@00307D28, func_00282D30@00307D50,
   func_002830C0@00307D68, func_00281DE8@00307D84, func_00280B90@00307DDC,
   func_00281DE8@00307DF0, func_00282D30@00307E0C, func_0027C768@00307EE8,
   func_00280B90@00307FC0]; data addresses materialised [0x190000, 0x1b0000,
   0x800000, 0x3c230000]; float constants [0x3f800000, 0x3f800000,
   0x3f800000, 0x3f800000, 0x3f800000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [LQ/SQ
   78820000@0x0050, LQ/SQ 7E420000@0x0054, LQ/SQ 78A20000@0x005c, LQ/SQ
   7E220000@0x0060]. Head: addiu $sp, $sp, -0x150 ; sd $s2, 0xF0($sp) ; sd
   $s6, 0x110($sp) ; addiu $s2, $sp, 0xC0 ; sd $s7, 0x118($sp) ; daddu $s6,
   $a2, $zero. Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
