/* func_L16_003C3870 - 412 bytes, 3 place(s), level 16_snivelak, retail at
   0x003c3870. Screened with tools/w10_screen.py: 15 call(s)
   [func_L00_003253A0@003C389C, func_00282CC0@003C38AC,
   func_00283B98@003C38B8, func_00283B98@003C38C8, func_00283B98@003C38D8,
   func_00277258@003C38F4, func_00282DB0@003C3918, func_00282F20@003C3940,
   func_00282DF0@003C3948, func_00282DB0@003C3958, func_00282D30@003C3974,
   func_00282CC0@003C3984, func_00282F20@003C399C, func_00282948@003C39BC,
   func_0029F338@003C39DC]; data addresses materialised [0x1c0000]; float
   constants [0x3fc00000, 0x3f800000, 0x3fa00000, 0x3f000000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 78620000@0x0098, LQ/SQ 7E420000@0x009c]. Head: addiu
   $sp, $sp, -0x40 ; sd $s1, 0x18($sp) ; sd $s2, 0x20($sp) ; daddu $s1, $a0,
   $zero ; sd $ra, 0x28($sp) ; swc1 $f20, 0x30($sp). Not matched in this
   wave: the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only
   through a `mode(TI)` local, and this cc1 folds the destination offset
   into the `sq` where retail materialises the pointer first. What would
   unblock it: the exact source expression tree, so this cc1 picks the same
   registers in the same order. */
