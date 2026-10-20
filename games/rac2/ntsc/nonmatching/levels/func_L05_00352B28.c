/* func_L05_00352B28 - 360 bytes, 2 place(s), level 05_feltzin_system,
   retail at 0x00352b28. Screened with tools/w10_screen.py: 4 call(s)
   [func_002B9030@00352B84, func_00283CF0@00352BB4, func_002A77E0@00352BD4,
   func_00283CE0@00352C14]; data addresses materialised [0x200000,
   0x350000]; float constants [0x40800000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 7A020000@0x0070, LQ/SQ 7C620000@0x0074]. Head: addiu $sp, $sp,
   -0x70 ; sd $s0, 0x0($sp) ; sd $s2, 0x10($sp) ; daddu $s0, $a0, $zero ; sd
   $s3, 0x18($sp) ; daddu $s2, $a2, $zero. Not matched in this wave: the
   128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
