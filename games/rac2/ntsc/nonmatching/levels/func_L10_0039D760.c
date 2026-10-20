/* func_L10_0039D760 - 604 bytes, 2 place(s), level 10_hrugis_cloud, retail
   at 0x0039d760. Screened with tools/w10_screen.py: 11 call(s)
   [func_00282978@0039D7C4, func_002A7878@0039D7F4, func_002A7878@0039D824,
   func_002A7878@0039D83C, func_00282CF0@0039D858, func_00282F20@0039D86C,
   func_00282CC0@0039D87C, func_L00_0031AA28@0039D8D4,
   func_002A78D8@0039D91C, func_002A78D8@0039D934, func_002A78D8@0039D94C];
   data addresses materialised [0x1b0000, 0x3dcc0000, 0x230000]; float
   constants [0x1b0000, 0x3f000000, 0x3f800000, 0x230000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 7AA20000@0x0088, LQ/SQ 7FA20000@0x008c, COP1
   46800020@0x0148, LQ/SQ 7A420000@0x017c, LQ/SQ 7E020000@0x0180, LQ/SQ
   7A020000@0x01b0, LQ/SQ 7E220000@0x01b4, LQ/SQ 7A220000@0x0200, LQ/SQ
   7E020000@0x0204]. Head: addiu $sp, $sp, -0xA0 ; sd $s2, 0x50($sp) ; sd
   $s4, 0x60($sp) ; daddu $s2, $zero, $zero ; sd $fp, 0x80($sp) ; addiu $s4,
   $sp, 0x10. Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
