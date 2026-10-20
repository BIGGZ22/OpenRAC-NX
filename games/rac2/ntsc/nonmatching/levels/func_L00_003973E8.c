/* func_L00_003973E8 - 1488 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003973e8. Screened with tools/w10_screen.py: 39 call(s)
   [func_0027C878@00397444, func_0027C878@0039748C, func_00282CF0@00397518,
   func_00282CF0@0039752C, func_00282DD8@00397540, func_00282DF0@0039754C,
   func_00282D30@00397584, func_00282CC0@00397594, func_00282CF0@003975A4,
   func_00282CF0@003975DC, func_00282CF0@003975EC, func_00282DD8@003975FC,
   func_00282DF0@00397604, func_00282D30@0039763C, func_00282CC0@0039764C,
   func_00282CF0@0039765C, func_00280B90@003976BC, func_00282CF0@00397740,
   func_00282CF0@00397750, func_00282DF0@00397758, func_00282DF0@00397764,
   func_00282D30@003977A8, func_00282D30@003977E4, func_00282CC0@003977F8,
   func_00282CC0@0039780C, func_00282CF0@0039781C, func_00282D30@0039782C,
   func_00282CF0@0039783C, func_00282CC0@0039784C, func_00282CF0@0039785C,
   func_00282DD8@0039786C, func_00282DF0@00397874, func_00282D30@003978A8,
   func_00282CC0@003978B8, func_00282CF0@003978C8, func_00282CC0@003978D8,
   func_00282CF0@003978E8, func_00280B90@00397940, func_002FC560@00397970];
   data addresses materialised [0x1c0000, 0x1c0000, 0x1c0000, 0x50000];
   float constants [0x3f800000, 0x3f800000, 0x3f800000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 7A420000@0x02dc, LQ/SQ 7FA20000@0x02e0, LQ/SQ
   7A620000@0x02e8, LQ/SQ 7C620000@0x02ec, LQ/SQ 7A620000@0x03a4, LQ/SQ
   7E420000@0x03a8, LQ/SQ 7A420000@0x03e0, LQ/SQ 7E620000@0x03e4]. Head:
   addiu $sp, $sp, -0x250 ; sd $s6, 0x210($sp) ; daddu $s6, $a0, $zero ; sd
   $s4, 0x200($sp) ; swc1 $f23, 0x248($sp) ; daddu $s4, $a1, $zero. Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
