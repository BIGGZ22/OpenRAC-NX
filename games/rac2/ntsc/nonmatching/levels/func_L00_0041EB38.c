/* func_L00_0041EB38 - 800 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041eb38. Screened with tools/w10_screen.py: 18 call(s)
   [func_0029EFD0@0041EB70, func_00282F20@0041EBF8, func_002AB1F0@0041EC18,
   func_002AFFD8@0041EC34, func_00282D30@0041EC44, func_00282CC0@0041EC54,
   func_00282CF0@0041EC68, func_00282F20@0041EC78,
   func_L00_0037FA90@0041ECAC, func_00282F20@0041ECC0,
   func_00283CF0@0041ECEC, func_00283CE0@0041ECFC, func_002AB1F0@0041ED3C,
   func_002B4038@0041ED80, func_0029F338@0041ED90, func_002FED90@0041EDF8,
   func_L00_002E6EF8@0041EE14, func_002A0F48@0041EE1C]; data addresses
   materialised [0x190000, 0x140000, 0x260000, 0x3c880000, 0x3eb20000,
   0x190000, 0x420000]; float constants [0x41f00000, 0x41f00000, 0x3f800000,
   0x3f800000]; $gp-relative small data: yes   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7A620000@0x00d8,
   LQ/SQ 7FA20000@0x00dc, LQ/SQ 7AC20000@0x0194, LQ/SQ 7C620000@0x0198,
   LQ/SQ 7A020000@0x0298, LQ/SQ 7FA20000@0x029c, LQ/SQ 78620000@0x02a8,
   LQ/SQ 7E020000@0x02ac, LQ/SQ 7BA20000@0x02cc, LQ/SQ 7E020000@0x02d0].
   Head: addiu $sp, $sp, -0x80 ; sd $s6, 0x50($sp) ; sd $s5, 0x48($sp) ;
   daddu $s6, $a0, $zero ; sd $s7, 0x58($sp) ; daddu $s5, $a2, $zero. Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
