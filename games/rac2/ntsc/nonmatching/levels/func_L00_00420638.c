/* func_L00_00420638 - 324 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00420638. Screened with tools/w10_screen.py: 13 call(s)
   [func_00334730@0042066C, func_00282DB0@0042067C, func_00282D30@00420694,
   func_00282CF0@004206A4, func_00282CC0@004206BC, func_00282D30@004206D4,
   func_00282CC0@004206E4, func_00282CC0@004206F4, func_00282C88@004206FC,
   func_00282C88@00420728, func_00282C88@00420730, func_00282D30@00420740,
   func_00282CC0@00420750]; data addresses materialised [0x1c0000]; float
   constants [0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7A020000@0x0078, LQ/SQ 7CC20000@0x007c, LQ/SQ 7A220000@0x00e8, LQ/SQ
   7C620000@0x00ec]. Head: addiu $sp, $sp, -0x70 ; daddu $a1, $zero, $zero ;
   sd $s0, 0x30($sp) ; sd $s1, 0x38($sp) ; sd $s3, 0x48($sp) ; sd $s4,
   0x50($sp). Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
