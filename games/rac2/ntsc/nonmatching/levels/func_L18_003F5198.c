/* func_L18_003F5198 - 572 bytes, 3 place(s), level 18_damosel, retail at
   0x003f5198. Screened with tools/w10_screen.py: 8 call(s)
   [func_00283CF0@003F51B8, func_00283CF0@003F51C8, func_002B60B0@003F52D8,
   func_002A7750@003F5324, func_002A7750@003F534C,
   func_L18_003F7590@003F5364, func_002A7750@003F5384,
   func_L18_0032B768@003F53B4]; data addresses materialised [0x3efa0000,
   0x3ecc0000, 0x3fb30000, 0x3e4c0000, 0x3bb60000, 0x3fc90000, 0x3b910000,
   0x3e080000, 0x3b640000, 0x3e560000, 0x1b0000]; float constants
   [0x41700000, 0x40c00000, 0x41800000, 0x3f000000, 0x40000000, 0x40a00000];
   $gp-relative small data: no   saves: -; callee-saved registers saved [-];
   words plain C cannot emit [LQ/SQ 78620000@0x01fc, LQ/SQ 7C820000@0x0200,
   LQ/SQ 78620000@0x020c, LQ/SQ 7CA20000@0x0210]. Head: addiu $sp, $sp,
   -0x20 ; lui $at, 0x4170 ; mtc1 $at, $f12 ; sd $s1, 0x8($sp) ; sd $s2,
   0x10($sp) ; daddu $s1, $a0, $zero. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
