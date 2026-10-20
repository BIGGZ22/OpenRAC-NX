/* func_L00_00420388 - 280 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x00420388. Screened with tools/w10_screen.py: 8 call(s)
   [func_L00_002DB350@004203BC, func_00282CF0@004203F4,
   func_00282DF0@004203FC, func_00282DF0@00420408, func_00282D30@0042043C,
   func_00282D30@0042044C, func_00282CC0@0042045C, func_00282CC0@0042046C];
   data addresses materialised [0x1b0000, 0x1c0000]; float constants
   [0x3f800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7A420000@0x0030,
   LQ/SQ 7FA20000@0x0038]. Head: addiu $sp, $sp, -0x70 ; sd $s2, 0x30($sp) ;
   swc1 $f22, 0x60($sp) ; daddu $s2, $a0, $zero ; sd $s0, 0x20($sp) ; mov.s
   $f22, $f12. Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
