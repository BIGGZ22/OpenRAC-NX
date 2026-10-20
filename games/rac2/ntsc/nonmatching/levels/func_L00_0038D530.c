/* func_L00_0038D530 - 284 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038d530. Screened with tools/w10_screen.py: 6 call(s)
   [func_0029EFD0@0038D558, func_002A7878@0038D5CC, func_002A7878@0038D5DC,
   func_002A7878@0038D5EC, func_002A0F48@0038D5FC, func_002FED90@0038D618];
   data addresses materialised [0xc0490000, 0x40490000]; float constants
   [0x3f800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7A420000@0x005c,
   LQ/SQ 7CA20000@0x0060, LQ/SQ 7A620000@0x0064, LQ/SQ 7E020000@0x0068].
   Head: addiu $sp, $sp, -0x40 ; sd $s2, 0x10($sp) ; sd $s3, 0x18($sp) ;
   daddu $s2, $a0, $zero ; sd $s1, 0x8($sp) ; daddu $s3, $a1, $zero. Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
