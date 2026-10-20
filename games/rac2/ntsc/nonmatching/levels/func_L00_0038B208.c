/* func_L00_0038B208 - 1260 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0038b208. Screened with tools/w10_screen.py: 4 call(s)
   [func_0027C878@0038B498, func_00283CF0@0038B5E0, func_00282D30@0038B60C,
   func_00280B90@0038B6A0]; data addresses materialised [0x190000, 0x800000,
   0x400000, 0x3f330000, 0x290000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000,
   0x50800000]; float constants [0x3f800000, 0x3f800000, 0x3fa00000,
   0x3f800000, 0x1b0000, 0x3f000000, 0x3f800000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [LQ/SQ 78620000@0x01c4, LQ/SQ 7EE20000@0x01c8, LQ/SQ
   78620000@0x01d4, LQ/SQ 7C820000@0x01d8, LQ/SQ 78A20000@0x01e4, LQ/SQ
   7C620000@0x01e8, COP1 46800860@0x0228, COP1 46800020@0x0250, LQ/SQ
   78C20000@0x02dc, LQ/SQ 7C620000@0x02e0, LQ/SQ 7FA600C0@0x03c8, COP1
   46806320@0x03d4, LQ/SQ 7BA600C0@0x0480]. Head: addiu $sp, $sp, -0x140 ;
   sd $s0, 0xD0($sp) ; sd $s1, 0xD8($sp) ; sd $s2, 0xE0($sp) ; sd $s3,
   0xE8($sp) ; sd $s4, 0xF0($sp). Not matched in this wave: the 128-bit copy
   needs an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)`
   local, and this cc1 folds the destination offset into the `sq` where
   retail materialises the pointer first. What would unblock it: the exact
   source expression tree, so this cc1 picks the same registers in the same
   order. */
