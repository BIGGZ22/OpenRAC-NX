/* func_L16_004051A8 - 736 bytes, 2 place(s), level 16_snivelak, retail at
   0x004051a8. Screened with tools/w10_screen.py: 18 call(s)
   [func_002A8C28@0040520C, func_002A7750@0040521C, func_00282CC0@0040524C,
   func_002A8C00@0040526C, func_002761F8@0040528C, func_00277258@004052B0,
   func_00282D30@00405304, func_002A8C00@00405340, func_00277F48@00405374,
   func_002A7750@00405384, func_00282D30@004053AC, func_00282C88@004053B4,
   func_L16_00406080@004053F0, func_002E5CF8@00405400,
   func_0029F338@00405408, func_002ACEC0@00405434, func_002ACEC0@0040544C,
   func_L00_002E6EF8@0040545C]; data addresses materialised [0x3f190000,
   0x10000, 0x1c0000, 0x3e990000, 0x1c0000, 0x45af0000, 0x10000, 0x3f190000,
   0x3c8e0000, 0x400000]; float constants [0x3fc00000, 0x40400000,
   0x3f800000, 0xbf000000, 0x3e800000]; $gp-relative small data: no   saves:
   -; callee-saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7A620000@0x0130, LQ/SQ 7E020000@0x0134, LQ/SQ 7A620000@0x0148, LQ/SQ
   7E020000@0x014c, LQ/SQ 78620000@0x01f0, LQ/SQ 7E820000@0x01f4]. Head:
   addiu $sp, $sp, -0xD0 ; sd $s2, 0xA0($sp) ; sd $s0, 0x90($sp) ; daddu
   $s2, $a0, $zero ; sd $s1, 0x98($sp) ; sd $s4, 0xB0($sp). Not matched in
   this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it
   only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
