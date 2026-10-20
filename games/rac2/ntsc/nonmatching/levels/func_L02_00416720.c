/* func_L02_00416720 - 224 bytes, 2 place(s), level 02_maktar_nebula, retail
   at 0x00416720. Screened with tools/w10_screen.py: 2 call(s)
   [func_002A7750@00416758, func_L02_003A6FE8@00416794]; data addresses
   materialised [-]; float constants [0x40000000, 0x3f800000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 78820000@0x0048, LQ/SQ 7C620000@0x004c]. Head: addiu
   $sp, $sp, -0x30 ; sd $s0, 0x0($sp) ; sd $s1, 0x8($sp) ; daddu $s0, $a1,
   $zero ; swc1 $f21, 0x28($sp) ; daddu $s1, $a0, $zero. Not matched in this
   wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only
   through a `mode(TI)` local, and this cc1 folds the destination offset
   into the `sq` where retail materialises the pointer first. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
