/* func_L00_0041E958 - 476 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041e958. Screened with tools/w10_screen.py: 5 call(s)
   [func_L00_002B39F0@0041EA00, func_002AF580@0041EA80,
   func_00283248@0041EAC0, func_00282CF0@0041EAD4,
   func_L00_002D3B58@0041EB04]; data addresses materialised [0x190000];
   float constants [0x3f000000, 0x3f800000, 0x190000, 0x190000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 78820000@0x0048, LQ/SQ 7FA20000@0x004c,
   LQ/SQ 78820000@0x00bc, LQ/SQ 7FA20000@0x00c0, LQ/SQ 7BA20000@0x0198,
   LQ/SQ 7C620000@0x019c]. Head: addiu $sp, $sp, -0x60 ; lui $v0, 0x19 ; sd
   $s2, 0x30($sp) ; addiu $a0, $v0, -0x61E0 ; swc1 $f23, 0x58($sp) ; daddu
   $s2, $v0, $zero. Not matched in this wave: the 128-bit copy needs an
   `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local, and
   this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
