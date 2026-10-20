/* func_L18_00427688 - 1212 bytes, 3 place(s), level 18_damosel, retail at
   0x00427688. Screened with tools/w10_screen.py: 17 call(s)
   [func_002A7878@0042781C, func_002B60B0@0042797C, func_002A7878@00427994,
   func_002A7820@004279F4, func_L09_0033E7A0@00427A00,
   func_L09_0033E7A0@00427A08, func_L09_0033E7A0@00427A10,
   func_L09_0033E7A0@00427A18, func_L09_0033E7A0@00427A20,
   func_L18_00427130@00427A38, func_L18_00426F78@00427A50,
   func_L18_00427010@00427A58, 0x1163b0@00427AAC,
   func_L18_00426F28@00427AC4, func_002A7750@00427ADC,
   func_002A7750@00427B08, func_002A7750@00427B20]; data addresses
   materialised [0x1a0000, 0x220000, 0x3ecc0000, 0x3b880000, 0x3e3b0000,
   0x3daa0000, 0x3c080000, 0x3f330000, 0x3f9c0000, 0x3c8e0000, 0x39910000,
   0x3f8c0000, 0x39910000, 0x3c880000, 0x36a20000, 0x39980000, 0x3c8e0000];
   float constants [0x3f000000, 0x3f400000, 0x41a00000, 0x41c80000,
   0x3f800000, 0x41a00000, 0x42200000, 0x40a00000, 0x3f800000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 78620000@0x0340, LQ/SQ 7C820000@0x0344]. Head: addiu
   $sp, $sp, -0x30 ; addiu $v1, $zero, 0xFF ; sd $s1, 0x8($sp) ; sd $s0,
   0x0($sp) ; daddu $s1, $a0, $zero ; sd $ra, 0x10($sp). Not matched in this
   wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only
   through a `mode(TI)` local, and this cc1 folds the destination offset
   into the `sq` where retail materialises the pointer first. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
