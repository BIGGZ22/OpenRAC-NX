/* func_L05_00379FA0 - 628 bytes, 4 place(s), level 05_feltzin_system,
   retail at 0x00379fa0. Screened with tools/w10_screen.py: 10 call(s)
   [func_002ED688@0037A058, func_0029FCC0@0037A090, func_00282F20@0037A0C4,
   func_00282CC0@0037A0D8, func_00282CF0@0037A0F4, func_00282F20@0037A108,
   func_00282CC0@0037A118, func_00282F20@0037A164, func_00282CC0@0037A174,
   func_L05_0037DA58@0037A188]; data addresses materialised [0x220000,
   0x3f4c0000]; float constants [0x40000000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 78620000@0x0100, LQ/SQ 7EA20000@0x0104, LQ/SQ 78620000@0x0114,
   LQ/SQ 7FA20000@0x0118, LQ/SQ 7A220000@0x0144, LQ/SQ 7C820000@0x0148].
   Head: addiu $sp, $sp, -0xB0 ; lui $v0, 0x22 ; sd $s1, 0x48($sp) ; addiu
   $v0, $v0, 0x1DD0 ; daddu $s1, $a0, $zero ; sd $s0, 0x40($sp). Not matched
   in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches
   it only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
