/* func_L18_003F7818 - 364 bytes, 3 place(s), level 18_damosel, retail at
   0x003f7818. Screened with tools/w10_screen.py: 8 call(s)
   [func_00282F20@003F78C0, func_00282CC0@003F78D0, func_00282F20@003F78EC,
   func_00282F20@003F7900, func_00282CC0@003F7910, func_00283248@003F792C,
   func_002A7750@003F7944, func_L18_003F8E30@003F794C]; data addresses
   materialised [-]; float constants [0xbf000000, 0x3f000000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [COP1 46800020@0x00a4]. Head: addiu $sp, $sp, -0x70 ; addiu
   $v0, $zero, 0x1 ; sd $s2, 0x20($sp) ; sd $s3, 0x28($sp) ; daddu $s2, $a0,
   $zero ; sd $s7, 0x48($sp). Not matched in this wave: the float code has
   to come out of the same expression tree to keep the FPU registers. What
   would unblock it: the exact source expression tree, so this cc1 picks the
   same registers in the same order. */
