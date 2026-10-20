/* func_L10_0039DA48 - 1492 bytes, 2 place(s), level 10_hrugis_cloud, retail
   at 0x0039da48. Screened with tools/w10_screen.py: 39 call(s)
   [func_0027C878@0039DAB8, func_0027C878@0039DB04, func_00282CF0@0039DBA0,
   func_00282CF0@0039DBB4, func_00282DD8@0039DBC8, func_00282DF0@0039DBD4,
   func_00282D30@0039DC04, func_00282CC0@0039DC14, func_00282CF0@0039DC24,
   func_00282CF0@0039DC54, func_00282CF0@0039DC64, func_00282DD8@0039DC74,
   func_00282DF0@0039DC7C, func_00282D30@0039DCA8, func_00282CC0@0039DCB8,
   func_00282CF0@0039DCC8, func_00280B90@0039DD1C, func_00282CF0@0039DDA0,
   func_00282CF0@0039DDB0, func_00282DF0@0039DDB8, func_00282DF0@0039DDC4,
   func_00282D30@0039DE08, func_00282D30@0039DE44, func_00282CC0@0039DE58,
   func_00282CC0@0039DE6C, func_00282CF0@0039DE7C, func_00282D30@0039DE8C,
   func_00282CF0@0039DE9C, func_00282CC0@0039DEAC, func_00282CF0@0039DEBC,
   func_00282DD8@0039DECC, func_00282DF0@0039DED4, func_00282D30@0039DF08,
   func_00282CC0@0039DF18, func_00282CF0@0039DF28, func_00282CC0@0039DF38,
   func_00282CF0@0039DF48, func_00280B90@0039DFA0, func_002FC560@0039DFD0];
   data addresses materialised [0x1c0000, 0x1c0000, 0x1c0000, 0x50000];
   float constants [0x3f800000, 0x3f800000]; $gp-relative small data: no
   saves: -; callee-saved registers saved [-]; words plain C cannot emit
   [LQ/SQ 7A420000@0x02dc, LQ/SQ 7FA20000@0x02e0, LQ/SQ 7A620000@0x02e8,
   LQ/SQ 7C620000@0x02ec, LQ/SQ 7A620000@0x03a4, LQ/SQ 7E220000@0x03a8,
   LQ/SQ 7A220000@0x03e0, LQ/SQ 7E620000@0x03e4]. Head: addiu $sp, $sp,
   -0x270 ; sd $fp, 0x230($sp) ; daddu $fp, $a0, $zero ; sd $s0, 0x1F0($sp)
   ; sd $s1, 0x1F8($sp) ; addiu $v0, $fp, 0x10. Not matched in this wave:
   the 128-bit copy needs an `lq`/`sq` pair; plain C reaches it only through
   a `mode(TI)` local, and this cc1 folds the destination offset into the
   `sq` where retail materialises the pointer first. What would unblock it:
   the exact source expression tree, so this cc1 picks the same registers in
   the same order. */
