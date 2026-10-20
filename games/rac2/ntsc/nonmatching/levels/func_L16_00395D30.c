/* func_L16_00395D30 - 412 bytes, 2 place(s), level 16_snivelak, retail at
   0x00395d30. Screened with tools/w10_screen.py: 15 call(s)
   [func_00335438@00395D90, func_002833F0@00395DA4, func_00283698@00395DB4,
   func_002A91F0@00395DC4, func_00282CF0@00395DD8, func_00283098@00395DEC,
   func_00282F20@00395E04, func_00282CC0@00395E14, func_00282CF0@00395E28,
   func_00282F20@00395E3C, func_00282DD8@00395E58, func_00283098@00395E6C,
   func_00283098@00395E80, func_002AB6D8@00395E90, func_002A91F0@00395EA0];
   data addresses materialised [0x1c0000, 0x1b0000]; float constants
   [0x1b0000, 0x1b0000, 0x1b0000, 0x40400000, 0x3f800000, 0x1b0000,
   0x1b0000, 0x1b0000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 78620000@0x011c,
   LQ/SQ 7CC20000@0x0120]. Head: addiu $sp, $sp, -0x100 ; addiu $v0, $zero,
   -0x1 ; sd $s5, 0xE8($sp) ; sd $s0, 0xC0($sp) ; daddu $s5, $a0, $zero ; sd
   $s1, 0xC8($sp). Not matched in this wave: the 128-bit copy needs an
   `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local, and
   this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
