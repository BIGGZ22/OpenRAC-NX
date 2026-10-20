/* MMI (SPECIAL2) instruction: no C spelling. 104 bytes, level 04 (1 place).
   Retail at 0x00422D40 carries the SPECIAL2 word 0x700024A9 at +0x14 - opcode 0x1C, which
   rabbitizer prints as `INVALID`; it is the same family the near miss of func_L00_003A8E28
   documents (words 0x700014A9 / 0x700024A9, opcode 0x1C with the MMI funct field), one of the
   packed helpers the lot rows func_L00_00428000, func_L05_003D0538 and func_L00_0040B2C0 share.
   The body builds four float constants (0x479C4000, 0xC788B800, 0x4808B800, 0x3F800000), spills
   them into a 16-byte stack slot with swc1 and then feeds the packed word and the slot to the
   MMI op. ASM_FUNC candidate, as for the rest of the family. */
