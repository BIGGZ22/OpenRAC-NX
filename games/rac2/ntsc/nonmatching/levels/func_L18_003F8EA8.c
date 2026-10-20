/* func_L18_003F8EA8 - 244 bytes, 3 place(s), level 18_damosel, retail at
   0x003f8ea8. Screened with tools/w10_screen.py: 2 call(s)
   [func_L02_002F21C0@003F8EF8, func_L02_002F21C0@003F8F40]; data addresses
   materialised [0x200000]; float constants [-]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 78620000@0x0040, LQ/SQ 7CA20000@0x0044]. Head: addiu $sp, $sp,
   -0x30 ; lui $v0, 0x20 ; sd $s0, 0x0($sp) ; addiu $v0, $v0, -0x1B80 ; swc1
   $f20, 0x20($sp) ; daddu $s0, $a3, $zero. Not matched in this wave: the
   128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
