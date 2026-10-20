/* func_L13_0041B670 - 1004 bytes, 2 place(s), level 13_boldan, retail at
   0x0041b670. Screened with tools/w10_screen.py: 11 call(s)
   [func_L13_0041BA60@0041B740, func_002E5CF8@0041B774,
   func_L00_002D40A0@0041B794, func_002A8C28@0041B7C8,
   func_L00_002D40F0@0041B7D8, func_L00_002B9D60@0041B8D0,
   func_002E5CF8@0041B90C, func_00288CB0@0041B948,
   func_L00_002E6CF0@0041B980, func_002AA2E8@0041B998,
   func_002AA548@0041BA20]; data addresses materialised [0x2b0000, 0x190000,
   0x190000, 0x190000, 0x140000, 0x10000, 0x3ff90000, 0x420000, 0x1b0000,
   0x1a0000, 0x1a0000, 0x3e4c0000, 0x190000, 0x3ff90000, 0x3ff90000,
   0x420000, 0x3d880000, 0x39910000, 0x39910000, 0x3c880000]; float
   constants [0x1a0000, 0x3fc00000, 0x41a00000, 0x3f800000, 0x3f800000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 78620000@0x0230, LQ/SQ 7FA20000@0x0234].
   Head: addiu $sp, $sp, -0x50 ; sd $s3, 0x28($sp) ; sd $s0, 0x10($sp) ;
   daddu $s3, $a0, $zero ; sd $s1, 0x18($sp) ; sd $s4, 0x30($sp). Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
