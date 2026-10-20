/* func_L16_003C3A10 - 288 bytes, 3 place(s), level 16_snivelak, retail at
   0x003c3a10. Screened with tools/w10_screen.py: 7 call(s)
   [func_0029EFD0@003C3A34, func_002B37E8@003C3A4C, func_002A7878@003C3A5C,
   func_002A7A00@003C3AA4, func_002A7A00@003C3ACC, func_002A7878@003C3ADC,
   func_002A7820@003C3B04]; data addresses materialised [0x1b0000,
   0x3d4c0000, 0x3dee0000, 0x39980000, 0x1b0000, 0x1b0000, 0x39910000,
   0x1b0000]; float constants [-]; $gp-relative small data: no   saves: -;
   callee-saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7A420000@0x0068, LQ/SQ 7C820000@0x006c, LQ/SQ 7A620000@0x0074, LQ/SQ
   7C620000@0x0078]. Head: addiu $sp, $sp, -0x30 ; sd $s0, 0x0($sp) ; sd
   $s2, 0x10($sp) ; daddu $s0, $a0, $zero ; sd $s3, 0x18($sp) ; daddu $s2,
   $a1, $zero. Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
