/* func_L07_00323038 - 584 bytes, 4 place(s), level 07_siberius, retail at
   0x00323038. Screened with tools/w10_screen.py: 7 call(s)
   [func_L00_0032C638@00323064, func_00283248@0032308C,
   func_L01_00322288@00323098, func_002B5BC0@003230AC,
   func_0029F530@003230B4, func_L07_00321D68@00323100,
   func_002A88A0@00323174]; data addresses materialised [-]; float constants
   [0x42800000, 0xbf000000, 0x44160000, 0x42700000, 0x3f800000, 0x40a00000,
   0x3f800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7C400000@0x021c].
   Head: addiu $sp, $sp, -0x50 ; sd $s0, 0x10($sp) ; swc1 $f23, 0x48($sp) ;
   daddu $s0, $a0, $zero ; swc1 $f22, 0x40($sp) ; mov.s $f23, $f13. Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
