/* MMI (SPECIAL2) instruction: no C spelling. 184 bytes, level 11 (1 place).
   Retail at 0x00397A30 carries the SPECIAL2 word 0x700014A9, and it is literally the first
   instruction after the prologue - opcode 0x1C with the MMI funct field, the very word the near
   miss of func_L00_003A8E28 documents (0x700014A9 / 0x700024A9). The rest is ordinary: the body
   calls func_002A77E0(5), loops that count down writing two bound floats (0x3DCCCCCD, 0x3ADA740F)
   with func_002ACB48, then per iteration calls func_002A7820(-5, 5) and func_L02_003449D8 with
   the small-data pointer at $gp-0x338. ASM_FUNC candidate, as for the rest of the family. */
