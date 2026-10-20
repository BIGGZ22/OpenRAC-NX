/* func_L18_0035D5F8 - 344 bytes, 3 place(s), level 18_damosel, retail at
   0x0035d5f8. Screened with tools/w10_screen.py: 4 call(s)
   [func_L18_0035D238@0035D68C, func_00282C48@0035D6A8,
   func_00282E48@0035D6C0, func_00282E48@0035D6D0]; data addresses
   materialised [0x51ba0000, 0x3dcc0000]; float constants [-]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 7BA20000@0x00f4, LQ/SQ 7E820000@0x00f8]. Head: addiu
   $sp, $sp, -0x80 ; sd $s2, 0x20($sp) ; sd $s3, 0x28($sp) ; daddu $s2,
   $zero, $zero ; sd $s4, 0x30($sp) ; daddu $s3, $a1, $zero. Not matched in
   this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it
   only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
