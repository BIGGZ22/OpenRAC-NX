/* func_L18_003F6CA8 - 1260 bytes, 3 place(s), level 18_damosel, retail at
   0x003f6ca8. Screened with tools/w10_screen.py: 22 call(s)
   [func_0029F530@003F6D34, func_L07_00321D68@003F6D78,
   func_00282CF0@003F6DF0, func_00282F20@003F6E10, func_00283248@003F6E2C,
   func_002AAA00@003F6E4C, func_L07_00321D68@003F6E58,
   func_0029F530@003F6E84, func_00282D30@003F6EF0, func_00282CC0@003F6F00,
   func_L01_00321178@003F6F0C, func_002B5278@003F6F64,
   func_002B5258@003F6F74, func_002B4FF0@003F6F8C, func_002A7750@003F702C,
   func_L07_00321D68@003F7098, func_L00_002EE290@003F70A8,
   func_00282CC0@003F70EC, func_00283028@003F7100, func_00282D30@003F7118,
   func_00282CC0@003F7128, func_L07_00321D68@003F7168]; data addresses
   materialised [0x2c0000, 0x3c880000, 0x3b5a0000, 0x3b5a0000, 0x3a830000,
   0x1c0000, 0x3f4c0000]; float constants [0x40800000, 0x41b00000,
   0xbf800000, 0x40800000, 0x41200000, 0x41880000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [LQ/SQ 78620000@0x03a8, LQ/SQ 7FA20000@0x03ac]. Head: addiu $sp,
   $sp, -0x50 ; addiu $v0, $zero, -0x3 ; sd $s2, 0x20($sp) ; sd $s0,
   0x10($sp) ; daddu $s2, $a0, $zero ; sd $s3, 0x28($sp). Not matched in
   this wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it
   only through a `mode(TI)` local, and this cc1 folds the destination
   offset into the `sq` where retail materialises the pointer first. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
