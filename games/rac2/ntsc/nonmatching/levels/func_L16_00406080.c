/* func_L16_00406080 - 756 bytes, 3 place(s), level 16_snivelak, retail at
   0x00406080. Screened with tools/w10_screen.py: 14 call(s)
   [func_0029EFD0@004060D8, func_00282CF0@0040612C, func_00282F20@0040613C,
   func_002AFFD8@00406150, func_00282DD8@0040616C, func_00282F20@0040617C,
   func_00282DD8@0040618C, func_00283DB8@0040622C, func_00282E48@0040625C,
   func_00282CF0@004062BC, func_00282F20@004062D0, func_00282DB0@004062DC,
   func_002831B0@004062E4, func_L16_00406550@00406330]; data addresses
   materialised [0x1c0000, 0x3dcc0000, 0x3d990000, 0x3fc90000, 0x3ee80000];
   float constants [0x3f800000, 0x40800000, 0x40400000, 0x43000000,
   0x42400000, 0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7BC20000@0x0084, LQ/SQ 7E420000@0x0088, LQ/SQ 7BA20000@0x00d8, LQ/SQ
   7E220000@0x00dc, LQ/SQ 7A420000@0x0190, LQ/SQ 7C820000@0x0194]. Head:
   addiu $sp, $sp, -0xC0 ; sd $fp, 0xA0($sp) ; swc1 $f21, 0xB8($sp) ; daddu
   $fp, $a0, $zero ; sd $s6, 0x90($sp) ; mov.s $f21, $f12. Not matched in
   this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it
   only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
