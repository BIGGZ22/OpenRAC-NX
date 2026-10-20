/* func_L00_0038FC48 - 456 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038fc48. Screened with tools/w10_screen.py: 4 call(s)
   [func_002A9640@0038FCEC, func_00282948@0038FCF8, func_0029F338@0038FD34,
   func_00282948@0038FDB0]; data addresses materialised [0x1b0000, 0x10000];
   float constants [-]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 78C20000@0x0110,
   LQ/SQ 7C620000@0x0114]. Head: addiu $sp, $sp, -0x20 ; sd $s2, 0x10($sp) ;
   sd $s0, 0x0($sp) ; daddu $s2, $a0, $zero ; sd $s1, 0x8($sp) ; sd $ra,
   0x18($sp). Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
