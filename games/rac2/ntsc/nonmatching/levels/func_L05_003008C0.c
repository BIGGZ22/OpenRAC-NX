/* func_L05_003008C0 - 564 bytes, 7 place(s), level 05_feltzin_system,
   retail at 0x003008c0. Screened with tools/w10_screen.py: 4 call(s)
   [func_00283CF0@00300930, func_00283CF0@0030094C, func_00283CF0@00300964,
   func_00283CF0@00300980]; data addresses materialised [0x1a0000, 0x1a0000,
   0x1a0000, 0x10000000, 0x1a0000, 0x50000000, 0x1b0000, 0x1b0000, 0x1b0000,
   0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000, 0x1b0000]; float
   constants [0x41800000, 0x1b0000]; $gp-relative small data: no   saves: -;
   callee-saved registers saved [-]; words plain C cannot emit [LQ/SQ
   78820000@0x0144, LQ/SQ 7C620000@0x0148]. Head: addiu $sp, $sp, -0x80 ;
   swc1 $f22, 0x60($sp) ; swc1 $f20, 0x50($sp) ; lui $at, 0x4180 ; mtc1 $at,
   $f22 ; mov.s $f20, $f12. Not matched in this wave: the 128-bit copy needs
   an `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local,
   and this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
