/* func_L00_003A8670 - 356 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x003a8670. Screened with tools/w10_screen.py: 8 call(s)
   [func_L00_0032A088@003A8694, func_002A7940@003A86CC,
   func_002A78D8@003A86DC, func_002A7878@003A86FC, func_002A7820@003A870C,
   func_00283CE0@003A871C, func_002A7878@003A8754, func_002A7878@003A878C];
   data addresses materialised [0x190000, 0x3c8e0000, 0x3c880000]; float
   constants [0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7A220000@0x0054, LQ/SQ 7E020000@0x0058]. Head: addiu $sp, $sp, -0x30 ; sd
   $s1, 0x8($sp) ; daddu $s1, $a0, $zero ; sd $s0, 0x0($sp) ; sd $s2,
   0x10($sp) ; addiu $a0, $zero, 0x5CB. Not matched in this wave: the
   128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
