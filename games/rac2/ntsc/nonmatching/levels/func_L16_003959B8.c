/* func_L16_003959B8 - 584 bytes, 2 place(s), level 16_snivelak, retail at
   0x003959b8. Screened with tools/w10_screen.py: 16 call(s)
   [func_L00_002B39F0@00395A00, func_00282CF0@00395A20,
   func_00282DD8@00395A30, func_00282F20@00395A40, func_002AFDD8@00395A58,
   func_00282F70@00395A6C, func_00283098@00395A80, func_00282DB0@00395A8C,
   func_002AA2E8@00395ADC, func_002AA2E8@00395AFC, func_00282F20@00395B28,
   func_00282CC0@00395B38, func_00282CC0@00395B90, func_00282A88@00395BA4,
   func_00282CF0@00395BB4, func_002AC9F8@00395BC8]; data addresses
   materialised [0x190000, 0x3daa0000, 0x3ab60000, 0x3ab60000, 0x40c90000,
   0x190000, 0x3dcc0000]; float constants [0x3f800000, 0x3f000000,
   0x3f800000]; $gp-relative small data: no   saves: -; callee-saved
   registers saved [-]; words plain C cannot emit [LQ/SQ 7A220000@0x0054,
   LQ/SQ 7E620000@0x0058]. Head: addiu $sp, $sp, -0xB0 ; swc1 $f20,
   0xA0($sp) ; sd $s0, 0x50($sp) ; sd $s1, 0x58($sp) ; lui $s0, 0x19 ; sd
   $s3, 0x68($sp). Not matched in this wave: the 128-bit copy needs an
   `lq`/`sq` pair; plain C reaches it only through a `mode(TI)` local, and
   this cc1 folds the destination offset into the `sq` where retail
   materialises the pointer first. What would unblock it: the exact source
   expression tree, so this cc1 picks the same registers in the same order. */
