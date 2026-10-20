/* func_L02_00410450 - 2220 bytes, 7 place(s), level 02_maktar_nebula,
   retail at 0x00410450. Screened with tools/w10_screen.py: 30 call(s)
   [func_L02_00410E58@004104AC, func_L02_004114A0@004105D0,
   func_002E5CF8@00410604, func_L00_002D40A0@00410624,
   func_002A8C28@00410658, func_L00_002D40F0@004106CC,
   func_L00_002C42E0@0041076C, func_L00_002E6CF0@00410820,
   func_002AA2E8@00410838, func_002A7750@00410860,
   func_L00_002B9D60@004108F8, func_002E5CF8@0041092C,
   func_L02_00410D00@0041095C, func_002B3868@0041096C,
   func_L00_002E6CF0@004109FC, func_002AA2E8@00410A14,
   func_002AA548@00410A9C, func_L02_00410D00@00410AB4,
   func_002B3868@00410AC4, func_L02_00410D00@00410AE4,
   func_L00_002C8BA8@00410B34, func_002E5CF8@00410B68,
   func_L00_002E6EF8@00410B78, func_002B3868@00410B88,
   func_L00_00303530@00410C2C, func_L00_00303530@00410C40,
   func_L00_002B9D60@00410C78, func_L02_00410D00@00410C80,
   func_002A7750@00410C90, func_L00_002E6EF8@00410CA0]; data addresses
   materialised [0x2a0000, 0x190000, 0x140000, 0x10000, 0x39910000,
   0x39910000, 0x3ff90000, 0x190000, 0x410000, 0x3d880000, 0x1b0000,
   0x1a0000, 0x1a0000, 0x3e4c0000, 0x3ff90000, 0x3ff90000, 0x410000,
   0x3d880000, 0x39910000, 0x39910000, 0x3c880000, 0x190000, 0x3ca30000,
   0x3f990000, 0x410000, 0x410000, 0x190000, 0x3ca30000, 0xbe4c0000,
   0x3ff90000, 0x3e4c0000, 0x410000]; float constants [0x1a0000, 0x3fc00000,
   0x41a00000, 0x3f800000, 0x3f800000, 0x3e800000, 0x3f800000, 0x3f800000,
   0x3f800000, 0x3f800000]; $gp-relative small data: no   saves: -; callee-
   saved registers saved [-]; words plain C cannot emit [LQ/SQ
   78620000@0x047c, LQ/SQ 7FA20000@0x0480, LQ/SQ 78620000@0x07fc, LQ/SQ
   7FA20000@0x0800]. Head: addiu $sp, $sp, -0x50 ; sd $s2, 0x20($sp) ; sd
   $s0, 0x10($sp) ; daddu $s2, $a0, $zero ; sd $s1, 0x18($sp) ; sd $s3,
   0x28($sp). Not matched in this wave: the 128-bit copy needs an `lq`/`sq`
   pair; plain C reaches it only through a `mode(TI)` local, and this cc1
   folds the destination offset into the `sq` where retail materialises the
   pointer first. What would unblock it: the exact source expression tree,
   so this cc1 picks the same registers in the same order. */
