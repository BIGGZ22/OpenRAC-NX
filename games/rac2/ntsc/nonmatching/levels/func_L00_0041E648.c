/* func_L00_0041E648 - 784 bytes, 27 place(s), level 00_aranos_tutorial,
   retail at 0x0041e648. Screened with tools/w10_screen.py: 21 call(s)
   [func_L00_0032A088@0041E694, func_002FEE50@0041E6A8,
   func_0029F338@0041E6B0, func_002FC560@0041E6D4, func_00281DE8@0041E6F0,
   func_00283CE0@0041E734, func_00283CE0@0041E740, func_00283CE0@0041E74C,
   func_00283CF0@0041E754, func_00282CF0@0041E7B8, func_00282DD8@0041E7CC,
   func_00282F20@0041E7DC, func_00282CC0@0041E7EC, func_00282CF0@0041E7FC,
   func_00283CE0@0041E808, func_00282CF0@0041E858, func_00282DD8@0041E868,
   func_00282F20@0041E878, func_00282CC0@0041E898, func_00282CF0@0041E8B8,
   func_00280B90@0041E8C8]; data addresses materialised [0x50000]; float
   constants [0x3f000000, 0x3f800000, 0x3e800000]; $gp-relative small data:
   no   saves: -; callee-saved registers saved [-]; words plain C cannot
   emit [LQ/SQ 7A620000@0x0288, LQ/SQ 7FA20000@0x028c, LQ/SQ
   7A420000@0x0294, LQ/SQ 7C620000@0x0298]. Head: addiu $sp, $sp, -0x130 ;
   sd $s0, 0xC0($sp) ; sd $s1, 0xC8($sp) ; daddu $s0, $a0, $zero ; sd $s2,
   0xD0($sp) ; daddu $s1, $a2, $zero. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
