/* func_L05_003CEFA0 (160 bytes, 2 places) -- MMI + $gp, not C.
   Three macro words (`0x7A020000`, `0x7C820000`, `0x7C600000` = LQ/SQ 128-bit copies of
   the s2+0x38 doubleword into s1+0x38 through s1->0x68+0x10) and a `$gp+0xB78` float table
   indexed by the 0xAA halfword; the rest is ordinary (jal func_328508, the 1 stores to
   s1+0x31/s1+0x20, jal func_3CF9C8/func_3CFC28, the 0x2C multiply). ASM_FUNC or a later
   pass with the TI128 levers, not this wave. */
