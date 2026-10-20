/* func_L00_0038B888 - 504 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038b888. Screened with tools/w10_screen.py: 11 call(s)
   [func_002A7878@0038B900, func_002A77E0@0038B90C, func_002A7940@0038B920,
   func_002A7878@0038B940, func_002A7878@0038B958, func_002AF000@0038B96C,
   func_002A78D8@0038B980, func_002A78D8@0038B99C, func_002A78D8@0038B9B4,
   func_002C2210@0038B9F0, func_002C2210@0038BA38]; data addresses
   materialised [0x3dcc0000, 0x3e190000, 0x3b440000, 0x3eb20000, 0x3fc40000,
   0x3d4c0000, 0x7f200000, 0x3d8f0000, 0x7f7f0000, 0x3f330000]; float
   constants [0x40000000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7A220000@0x00ec, LQ/SQ 7E620000@0x00f0]. Head: addiu $sp, $sp, -0x90 ; sd
   $s1, 0x28($sp) ; sd $s0, 0x20($sp) ; daddu $s1, $a0, $zero ; sd $s2,
   0x30($sp) ; sd $s3, 0x38($sp). Not matched in this wave: the 128-bit copy
   needs an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)`
   local, and this cc1 folds the destination offset into the `sq` where
   retail materialises the pointer first. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
