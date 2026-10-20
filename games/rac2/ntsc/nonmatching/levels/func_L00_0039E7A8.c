/* func_L00_0039E7A8 - 872 bytes, 19 place(s), level 00_aranos_tutorial,
   retail at 0x0039e7a8. Screened with tools/w10_screen.py: 8 call(s)
   [func_L00_0042A500@0039E7F4, func_00282948@0039E8F4,
   func_002ACBC8@0039E90C, func_002A0430@0039E9B8, func_002A89A8@0039E9D8,
   func_00282C48@0039E9E8, func_002DF3F8@0039EA14, func_002B0A18@0039EA98];
   data addresses materialised [0x1b0000, 0x190000, 0x190000, 0x190000,
   0x190000, 0x190000, 0x190000, 0x220000, 0x1a0000, 0x220000, 0x220000,
   0x1a0000, 0x1a0000, 0x1a0000, 0x1a0000]; float constants [0x1a0000,
   0x1a0000, 0x3f000000, 0x40000000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [LQ/SQ
   78620000@0x021c, LQ/SQ 7FA20000@0x0220]. Head: addiu $sp, $sp, -0x30 ; sd
   $s0, 0x10($sp) ; sd $s1, 0x18($sp) ; daddu $s0, $a0, $zero ; sd $ra,
   0x20($sp) ; lbu $v0, 0x30($s0). Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
