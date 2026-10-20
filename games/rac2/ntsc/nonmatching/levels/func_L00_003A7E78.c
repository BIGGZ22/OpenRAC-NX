/* func_L00_003A7E78 - 484 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a7e78. Screened with tools/w10_screen.py: 12 call(s)
   [func_002B4038@003A7E9C, func_0029F338@003A7EAC, func_00300190@003A7ECC,
   func_L00_003A85A0@003A7ED4, func_00282CC0@003A7F1C,
   func_00282CC0@003A7F38, func_L00_003A8670@003A7F54,
   func_00282D08@003A7F70, func_L00_003A8670@003A7F7C,
   func_L00_003A80F8@003A8010, func_00282978@003A8020,
   func_L00_003A85A0@003A8038]; data addresses materialised [0x447f0000];
   float constants [0x3f000000, 0x40000000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 78620000@0x007c, LQ/SQ 7FA20000@0x0080, LQ/SQ 78620000@0x008c,
   LQ/SQ 7FA20000@0x0090]. Head: addiu $sp, $sp, -0x50 ; sd $s1, 0x28($sp) ;
   sd $s0, 0x20($sp) ; daddu $s1, $a0, $zero ; sd $s3, 0x38($sp) ; addiu
   $a0, $zero, 0x5CB. Not matched in this wave: the 128-bit copy needs an
   `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local, and
   this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
