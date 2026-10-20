/* func_L00_003A7C30 - 580 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a7c30. Screened with tools/w10_screen.py: 14 call(s)
   [func_002B4038@003A7C70, func_0029EFD0@003A7C88, func_003001E0@003A7CD0,
   func_00283CF0@003A7D1C, func_00283CE0@003A7D2C,
   func_L00_0032A088@003A7D54, func_002AFFD8@003A7D68,
   func_00282D30@003A7D7C, func_L00_003A8060@003A7D88,
   func_L00_003A8E28@003A7DC8, func_00282DF0@003A7DF4,
   func_00282F20@003A7E04, func_00282CC0@003A7E2C,
   func_L00_003A80F8@003A7E38]; data addresses materialised [0x140000,
   0x260000, 0x3c880000, 0x1c0000]; float constants [0x43000000, 0x3f800000,
   0xbf800000]; $gp-relative small data: yes   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7A620000@0x0088,
   LQ/SQ 7E820000@0x008c, LQ/SQ 7A020000@0x01b8, LQ/SQ 7C620000@0x01bc,
   LQ/SQ 7A620000@0x01ec, LQ/SQ 7E020000@0x01f0]. Head: addiu $sp, $sp,
   -0x60 ; addiu $v0, $zero, -0x1 ; sd $s0, 0x20($sp) ; sd $s3, 0x38($sp) ;
   daddu $s0, $a2, $zero ; sd $s5, 0x48($sp). Not matched in this wave: the
   128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
