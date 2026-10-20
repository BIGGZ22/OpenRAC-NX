/* func_L00_0041E168 - 832 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041e168. Screened with tools/w10_screen.py: 20 call(s)
   [func_00282CF0@0041E1C0, func_00282CF0@0041E204, func_00282F20@0041E21C,
   func_00282DB0@0041E228, func_00282D30@0041E238, func_00282CF0@0041E248,
   func_002A7878@0041E288, func_002A77E0@0041E294, func_002A7878@0041E2C0,
   func_002ACEC0@0041E2D4, func_002A7878@0041E2E0, func_00282F20@0041E2F0,
   func_002A7878@0041E2FC, func_002A78D8@0041E318, func_002A78D8@0041E330,
   func_002A78D8@0041E348, func_002C2210@0041E3C8, func_00283CF0@0041E3E4,
   func_002C2210@0041E434, func_00283CF0@0041E450]; data addresses
   materialised [0x1c0000, 0x1b0000, 0x190000, 0x140000, 0x260000,
   0x3c880000, 0x3e190000, 0x3f860000, 0xbf860000, 0x800000, 0x800000,
   0x200000, 0x3dcc0000, 0x7f000000, 0x3d8f0000, 0x7f7f0000, 0x3f330000];
   float constants [0x3f800000, 0x3f800000, 0x40800000, 0x40800000]; $gp-
   relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 7A420000@0x01a4, LQ/SQ 7EA20000@0x01a8].
   Head: addiu $sp, $sp, -0xC0 ; sd $s2, 0x60($sp) ; sd $s3, 0x68($sp) ;
   daddu $s2, $a0, $zero ; sd $s6, 0x80($sp) ; daddu $s3, $a1, $zero. Not
   matched in this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C
   reaches it only through a `mode(TI)` local, and this cc1 folds the
   destination offset into the `sq` where retail materialises the pointer
   first. What would unblock it: the exact source expression tree, so this
   cc1 picks the same registers in the same order. */
