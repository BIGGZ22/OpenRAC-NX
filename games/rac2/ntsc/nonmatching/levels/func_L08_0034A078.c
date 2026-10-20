/* func_L08_0034A078 - 296 bytes, 4 place(s), level 08_tabora, retail at
   0x0034a078. Screened with tools/w10_screen.py: 6 call(s)
   [func_002B9030@0034A0A4, func_00283CF0@0034A0C8, func_002A7878@0034A0F4,
   func_002A77E0@0034A100, func_002A7820@0034A120, func_00283CE0@0034A130];
   data addresses materialised [0x47430000, 0x200000]; float constants
   [0x40800000, 0x45fa0000, 0x3f800000, 0x7f0000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [LQ/SQ 7A020000@0x0040, LQ/SQ 7C620000@0x0044, LQ/SQ
   7A820000@0x00f0, LQ/SQ 7E020000@0x00f4]. Head: addiu $sp, $sp, -0x30 ; sd
   $s0, 0x0($sp) ; sd $s2, 0x10($sp) ; daddu $s0, $a0, $zero ; sd $s3,
   0x18($sp) ; daddu $s2, $a2, $zero. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
