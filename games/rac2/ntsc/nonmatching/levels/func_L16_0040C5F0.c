/* func_L16_0040C5F0 - 500 bytes, 2 place(s), level 16_snivelak, retail at
   0x0040c5f0. Screened with tools/w10_screen.py: 11 call(s)
   [func_002A7878@0040C630, func_002A7820@0040C654, func_002A7878@0040C684,
   func_00282D30@0040C714, func_00282D30@0040C724, func_00282CC0@0040C734,
   func_00282D30@0040C744, func_00282CC0@0040C754, func_00283098@0040C764,
   func_00282CC0@0040C774, func_002C6DD8@0040C78C]; data addresses
   materialised [0x1b0000, 0x39910000, 0x1b0000, 0x1b0000, 0x1b0000,
   0x1b0000]; float constants [0x3f800000, 0x3f800000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 78660000@0x010c, LQ/SQ 78670020@0x0114, LQ/SQ
   78620010@0x0118, LQ/SQ 7FA60030@0x011c, LQ/SQ 7FA20040@0x0120, LQ/SQ
   7FA70050@0x0128]. Head: addiu $sp, $sp, -0xC0 ; lwc1 $f12, 0x2600($gp) ;
   sd $s4, 0x80($sp) ; lui $at, 0x1B ; lwc1 $f13, 0x15F4($at) ; daddu $s4,
   $a0, $zero. Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
