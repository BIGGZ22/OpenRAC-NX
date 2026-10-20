/* func_L00_003A2258 - 420 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a2258. Screened with tools/w10_screen.py: 12 call(s)
   [func_00283CE0@003A22C8, func_00283CE0@003A22D4,
   func_L00_003A1F68@003A22EC, func_00282CC0@003A2310,
   func_00282CC0@003A2320, func_L00_003A2018@003A233C,
   func_002A7878@003A234C, func_002A7878@003A235C, func_002A7878@003A236C,
   func_00282F20@003A2388, func_00282CC0@003A239C,
   func_L00_003A2668@003A23B8]; data addresses materialised [0x3d230000];
   float constants [0xbf800000, 0x3f800000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 7AE20000@0x009c, LQ/SQ 7E420000@0x00a0, LQ/SQ 7BC20000@0x00a4,
   LQ/SQ 7E220000@0x00a8]. Head: addiu $sp, $sp, -0x160 ; swc1 $f22,
   0x150($sp) ; swc1 $f21, 0x148($sp) ; sd $s0, 0xF0($sp) ; sd $s1,
   0xF8($sp) ; daddu $s0, $sp, $zero. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
