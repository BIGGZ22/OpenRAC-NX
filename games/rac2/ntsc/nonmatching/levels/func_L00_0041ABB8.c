/* func_L00_0041ABB8 - 228 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041abb8. Screened with tools/w10_screen.py: 2 call(s)
   [func_002FEE50@0041AC0C, func_002FED90@0041AC28]; data addresses
   materialised [-]; float constants [0x3f800000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [LQ/SQ 78A20000@0x0034, LQ/SQ 7E020000@0x0038, LQ/SQ
   78C20000@0x0040, LQ/SQ 7C620000@0x0044]. Head: addiu $sp, $sp, -0x30 ; sd
   $s0, 0x0($sp) ; sd $s2, 0x10($sp) ; sd $s3, 0x18($sp) ; daddu $s2, $a0,
   $zero ; sd $ra, 0x20($sp). Not matched in this wave: the 128-bit copy
   needs an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)`
   local, and this cc1 folds the destination offset into the `sq` where
   retail materialises the pointer first. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
