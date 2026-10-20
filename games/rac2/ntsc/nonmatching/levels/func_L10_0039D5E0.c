/* func_L10_0039D5E0 - 324 bytes, 2 place(s), level 10_hrugis_cloud, retail
   at 0x0039d5e0. Screened with tools/w10_screen.py: 1 call(s)
   [func_0029EFD0@0039D648]; data addresses materialised [0x1b0000]; float
   constants [-]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7AC20000@0x00c0,
   LQ/SQ 7C620000@0x00c4, LQ/SQ 7AE20000@0x00cc, LQ/SQ 7C820000@0x00d0].
   Head: addiu $sp, $sp, -0x70 ; sd $s7, 0x38($sp) ; sd $s0, 0x0($sp) ;
   daddu $s7, $a0, $zero ; sd $s1, 0x8($sp) ; daddu $s0, $a3, $zero. Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
