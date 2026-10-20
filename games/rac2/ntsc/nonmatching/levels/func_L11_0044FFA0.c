/* func_L11_0044FFA0 - 904 bytes, 2 place(s), level 11_joba, retail at
   0x0044ffa0. Screened with tools/w10_screen.py: 22 call(s)
   [func_002A8C28@00450010, func_002A7750@00450020, func_00282CC0@00450050,
   func_002A8C00@00450070, func_002761F8@00450090, func_00277258@004500B4,
   func_002A8C28@00450108, func_002A7750@00450118, func_00282D30@00450144,
   func_00282F20@00450164, func_00282CC0@0045018C, func_002A7878@004501B0,
   func_002A7878@004501C8, func_002C65A0@004501F8, func_00282F20@00450210,
   func_00282CC0@00450220, func_002A7878@00450240, func_002A7878@00450258,
   func_002C65A0@00450288, func_0032C700@004502D0, func_0029F338@004502D8,
   func_L00_002E6EF8@004502F0]; data addresses materialised [0x3f190000,
   0x10000, 0x450000, 0x1c0000, 0x3e990000, 0x1c0000, 0x10000, 0x3f190000,
   0x484d0000, 0x3dcc0000, 0x3f990000, 0x3fb90000, 0x3e4c0000, 0xbcf50000,
   0x3f330000, 0x3f660000, 0x3e990000, 0x450000, 0x450000]; float constants
   [0xbf000000, 0x3f000000, 0x3e800000, 0x41a00000]; $gp-relative small
   data: no   saves: -; callee-saved registers saved [-]; words plain C
   cannot emit [LQ/SQ 78620000@0x018c, LQ/SQ 7E820000@0x0190]. Head: addiu
   $sp, $sp, -0xD0 ; sd $s2, 0x90($sp) ; sd $s0, 0x80($sp) ; daddu $s2, $a0,
   $zero ; sd $s1, 0x88($sp) ; sd $s4, 0xA0($sp). Not matched in this wave:
   the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through
   a `mode(TI)` local, and this cc1 folds the destination offset into the
   `sq` where retail materialises the pointer first. What would unblock it:
   the exact source expression tree, so this cc1 picks the same registers in
   the same order. */
