/* func_L18_003FD630 - 252 bytes, 3 place(s), level 18_damosel, retail at
   0x003fd630. Screened with tools/w10_screen.py: 8 call(s)
   [func_002ACB48@003FD680, func_002A7A00@003FD690, func_002A7A00@003FD6A0,
   func_002A7820@003FD6AC, func_002A7878@003FD6BC, func_002A7878@003FD6CC,
   func_L11_00397C08@003FD6E4, func_L11_00397E68@003FD704]; data addresses
   materialised [-]; float constants [-]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 78A20000@0x0034, LQ/SQ 7FC20000@0x0038, LQ/SQ 78A20000@0x0040,
   LQ/SQ 7C820000@0x0044]. Head: addiu $sp, $sp, -0xF0 ; sd $fp, 0xC8($sp) ;
   sd $s0, 0xC0($sp) ; daddu $fp, $sp, $zero ; sd $ra, 0xD0($sp) ; swc1
   $f21, 0xE8($sp). Not matched in this wave: the 128-bit copy needs an
   `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local, and
   this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
