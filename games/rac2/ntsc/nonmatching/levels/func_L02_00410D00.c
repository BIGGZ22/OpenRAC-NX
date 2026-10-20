/* func_L02_00410D00 - 344 bytes, 7 place(s), level 02_maktar_nebula, retail
   at 0x00410d00. Screened with tools/w10_screen.py: 4 call(s)
   [func_002833F0@00410DC4, func_00282F20@00410DD8, func_00282CC0@00410DEC,
   func_002833F0@00410E14]; data addresses materialised [0x3c8e0000,
   0x40190000, 0x3f990000, 0xbf860000, 0x3eb20000]; float constants [-];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 7A620000@0x0084, LQ/SQ 7C620000@0x0088].
   Head: addiu $sp, $sp, -0x70 ; sd $s0, 0x10($sp) ; sd $s2, 0x20($sp) ; sd
   $s3, 0x28($sp) ; addiu $s2, $zero, 0x2 ; swc1 $f25, 0x68($sp). Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
