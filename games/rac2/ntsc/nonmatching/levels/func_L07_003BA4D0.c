/* func_L07_003BA4D0 - 1804 bytes, 2 place(s), level 07_siberius, retail at
   0x003ba4d0. Screened with tools/w10_screen.py: 31 call(s)
   [func_00282D30@003BA558, func_00282CC0@003BA574, func_002761F8@003BA58C,
   func_002AB1F0@003BA62C, func_00282D30@003BA654, func_00282CC0@003BA664,
   func_00283248@003BA6A8, func_00282E80@003BA6B8, func_00283248@003BA6CC,
   func_00283C80@003BA6DC, func_00282E48@003BA6EC, func_00282CF0@003BA750,
   func_00282DB0@003BA760, func_00282C48@003BA76C, func_00282D30@003BA798,
   func_00282CF0@003BA7A8, func_00282DB0@003BA7B8, func_00282DF0@003BA7C4,
   func_002831B0@003BA7F4, func_00283248@003BA858, func_00283C80@003BA864,
   func_00282C48@003BA894, func_002761F8@003BA8D4, func_002AF000@003BA940,
   func_00282CC0@003BA950, func_00282E48@003BA95C, func_002831B0@003BA970,
   func_002AB1F0@003BA9A4, func_00283CE0@003BA9B8, func_002831B0@003BA9DC,
   func_002761F8@003BAAD4]; data addresses materialised [0x190000, 0x1c0000,
   0x200000, 0x200000, 0x200000, 0x1b0000, 0x190000, 0x3fc90000, 0x3f860000,
   0x3f860000, 0x190000, 0x190000, 0x3c8e0000, 0x3c8e0000, 0x1c0000,
   0x3e320000, 0x200000]; float constants [0x3f000000, 0x190000, 0x40200000,
   0x40000000, 0x40200000, 0x40000000, 0x3f000000, 0x3e000000]; $gp-relative
   small data: no   saves: -; callee-saved registers saved [-]; words plain
   C cannot emit [LQ/SQ 78A30000@0x0064, LQ/SQ 7FA30000@0x0084, LQ/SQ
   78620000@0x00d0, LQ/SQ 7E620000@0x00d4, LQ/SQ 78820000@0x014c, LQ/SQ
   7C620000@0x0150, LQ/SQ 7A620000@0x056c, LQ/SQ 7C620000@0x0570, LQ/SQ
   78620000@0x05d4, LQ/SQ 7E420000@0x05d8]. Head: addiu $sp, $sp, -0x320 ;
   lui $v0, 0x19 ; sd $s0, 0x290($sp) ; sd $s1, 0x298($sp) ; sd $s2,
   0x2A0($sp) ; addiu $s1, $sp, 0x10. Not matched in this wave: the 128-bit
   copy needs an `lq`/`sq` pair; plain C reaches it only through a
   `mode(TI)` local, and this cc1 folds the destination offset into the `sq`
   where retail materialises the pointer first. What would unblock it: the
   exact source expression tree, so this cc1 picks the same registers in the
   same order. */
