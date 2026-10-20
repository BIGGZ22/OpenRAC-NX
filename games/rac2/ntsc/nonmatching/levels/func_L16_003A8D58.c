/* func_L16_003A8D58 - 904 bytes, 3 place(s), level 16_snivelak, retail at
   0x003a8d58. Screened with tools/w10_screen.py: 18 call(s)
   [func_002AFFD8@003A8DA8, func_00282CC0@003A8DD0, func_00282F20@003A8E0C,
   func_002761F8@003A8E4C, func_00277258@003A8E70, func_002E5CF8@003A8E84,
   func_00283CF0@003A8E98, func_00283CF0@003A8EB0, func_00283CF0@003A8EC8,
   func_002ABBB8@003A8F28, func_00277F48@003A8F44, func_00282948@003A8F54,
   func_00283CF0@003A8F70, func_00283CF0@003A8F88, func_00283CF0@003A8FA0,
   func_002ABBB8@003A9008, func_0029F338@003A909C,
   func_L09_003AEE80@003A90AC]; data addresses materialised [0x10000,
   0x45af0000, 0x3e4c0000, 0x810000, 0x810000, 0x447f0000]; float constants
   [0x40a00000, 0x40800000, 0x41200000, 0x41100000, 0x3f800000, 0x40a00000,
   0x40800000, 0x41200000, 0x40000000, 0x41100000, 0x40000000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 7A820000@0x0040, LQ/SQ 7C620000@0x0044, LQ/SQ
   7A820000@0x005c, LQ/SQ 7E020000@0x0060, LQ/SQ 7AC20000@0x00a4, LQ/SQ
   7EA20000@0x00a8]. Head: addiu $sp, $sp, 0x70 ; nop ; addiu $sp, $sp,
   -0xD0 ; sd $s3, 0x98($sp) ; addiu $v1, $sp, 0x20 ; sd $s4, 0xA0($sp). Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
