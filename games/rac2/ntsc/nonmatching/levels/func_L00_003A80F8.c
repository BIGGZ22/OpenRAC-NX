/* func_L00_003A80F8 - 320 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a80f8. Screened with tools/w10_screen.py: 4 call(s)
   [func_00282F20@003A8168, func_002761F8@003A81D8,
   func_L00_003A8238@003A81F0, func_00334160@003A8208]; data addresses
   materialised [0x140000, 0x260000, 0x10000, 0x45af0000, 0x190000,
   0x190000, 0x1c0000]; float constants [0x3f800000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 7A220000@0x005c, LQ/SQ 7FA20000@0x0060, LQ/SQ
   78820000@0x00ac, LQ/SQ 7C620000@0x00b0, LQ/SQ 78620000@0x00c0, LQ/SQ
   7C820000@0x00c4]. Head: lui $v0, 0x14 ; addiu $sp, $sp, -0x70 ; lbu $v1,
   -0x6A81($v0) ; addiu $a3, $zero, 0xE0 ; sd $s0, 0x40($sp) ; lui $a2,
   0x26. Not matched in this wave: the 128-bit copy needs an `lq`/`sq` pair;
   plain C reaches it only through a `mode(TI)` local, and this cc1 folds
   the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
