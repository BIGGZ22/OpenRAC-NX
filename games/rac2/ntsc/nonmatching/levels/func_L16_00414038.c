/* func_L16_00414038 - 2132 bytes, 2 place(s), level 16_snivelak, retail at
   0x00414038. Screened with tools/w10_screen.py: 67 call(s)
   [func_L11_0045FFB0@00414074, func_00281DE8@00414088,
   func_00282CF0@004141D4, func_00282F20@004141E8, func_00282CC0@004141F8,
   func_00282DD8@00414208, func_00282F20@00414218, func_00282F20@0041422C,
   func_00282CC0@0041423C, func_00282CC0@0041424C, func_00282CF0@0041425C,
   func_00282CC0@0041426C, func_00282CF0@0041427C, func_00280B90@0041428C,
   func_00282CC0@004142D0, func_00282CC0@004142E0, func_00280B90@004142F0,
   func_00282F20@00414304, func_00283180@00414338, func_00283198@00414344,
   func_00283B98@004143F0, func_00281DE8@00414408, func_00282F20@00414474,
   func_00282F20@00414488, func_00282CC0@00414498, func_00282CC0@004144A8,
   func_00282CF0@004144B8, func_00282CC0@004144C8, func_00282CC0@004144D8,
   func_00282CC0@004144E8, func_00282CF0@004144F8, func_00280B90@00414508,
   func_00282F20@0041456C, func_00282F20@0041457C, func_00282CC0@0041458C,
   func_00282CC0@0041459C, func_00282CF0@004145AC, func_00282CC0@004145BC,
   func_00282CC0@004145CC, func_00282CC0@004145DC, func_00282CF0@004145EC,
   func_00280B90@004145FC, func_00282F20@00414660, func_00282F20@00414670,
   func_00282CC0@00414680, func_00282CC0@00414690, func_00282CF0@004146A0,
   func_00282CC0@004146B0, func_00282CC0@004146C0, func_00282CC0@004146D0,
   func_00282CF0@004146E0, func_00280B90@004146F0, func_00282F20@00414740,
   func_00282F20@00414750, func_00282CC0@00414760, func_00282CC0@00414770,
   func_00282CF0@00414780, func_00282F20@00414798, func_00282CC0@004147A8,
   func_00282CC0@004147B8, func_00282CC0@004147C8, func_00282CF0@004147D8,
   func_00280B90@004147E8, func_002A7940@004147F0, func_00281EB8@00414810,
   func_002A7940@00414828, func_00281EB8@0041484C]; data addresses
   materialised [0x3dcc0000, 0x1c0000, 0x1b0000, 0x800000, 0x3c8e0000,
   0x3da30000, 0x1b0000, 0xff0000, 0xbf190000, 0x3ecc0000, 0x80800000];
   float constants [0x3f800000, 0x40e00000, 0x40800000, 0xc0800000,
   0x3f800000, 0x3f000000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   7AC20000@0x027c, LQ/SQ 7FA20000@0x0280, LQ/SQ 7AE20000@0x0288, LQ/SQ
   7C620000@0x028c]. Head: addiu $sp, $sp, -0x160 ; sd $s0, 0x100($sp) ;
   daddu $a0, $sp, $zero ; sd $s1, 0x108($sp) ; daddu $s0, $t1, $zero ; sd
   $s2, 0x110($sp). Not matched in this wave: the 128-bit copy needs an
   `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local, and
   this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
