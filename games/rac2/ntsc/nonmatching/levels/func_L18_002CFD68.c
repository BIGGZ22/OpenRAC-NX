/* func_L18_002CFD68 - 352 bytes, 2 place(s), level 18_damosel, retail at
   0x002cfd68. Screened with tools/w10_screen.py: 12 call(s)
   [func_L00_002AF7D0@002CFD94, func_L00_002B1F90@002CFDA4,
   func_L00_00322EC8@002CFDB4, func_L00_00322EC8@002CFDBC,
   func_L00_002D2E68@002CFDE4, func_L00_002A6708@002CFDEC,
   func_L00_002A53B8@002CFDF4, func_L18_002D0850@002CFE04,
   func_L00_002D24B8@002CFE38, func_002A677C@002CFE40,
   func_L18_002E0070@002CFE84, func_L00_002D2168@002CFEA0]; data addresses
   materialised [0x190000]; float constants [0x3f800000, 0x40a00000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 78620000@0x00f0, LQ/SQ 7C820000@0x00f4,
   LQ/SQ 78A20000@0x0100, LQ/SQ 7C620000@0x0104]. Head: addiu $sp, $sp,
   -0x30 ; sd $s0, 0x0($sp) ; sd $s1, 0x8($sp) ; lui $s0, 0x19 ; sd $s2,
   0x10($sp) ; daddu $s1, $a0, $zero. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
