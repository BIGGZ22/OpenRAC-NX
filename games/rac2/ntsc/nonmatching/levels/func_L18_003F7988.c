/* func_L18_003F7988 (124 bytes, 3 places) -- MMI, not C.
   Four LQ/SQ macro words (0x78A20000/0x7CC20000 = the s0+0x10 block into the record's
   +0x210 block; 0x78620000/0x7CA20000 = the record's +0x220 block into s0+0xF0) around
   a `lh 0x24($v1)` -> `cvt.s.w` store of a halfword as a float, then jal func_309158 and
   jal func_335668(s0, 0, -1). ASM_FUNC or the TI128 levers. */
