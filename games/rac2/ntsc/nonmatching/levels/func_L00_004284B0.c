/* 128-bit block moves: no C spelling. 176 bytes, level 00 (27 places).
   Retail at 0x4284B0 uses five raw 128-bit moves around its float work: `lq`/`sq` words 0x78620000,
   0x7CA20000, 0x7C820000, 0x78A20000, 0x7C620000 (opcodes 0x1E/0x1F, printed as .word by the
   disassembler) at +0x28 onwards, copying 16-byte blocks from the object's fields (0x70/0xB0,
   0x80/0xB8) while adding 2.0f to a float field and calling func_2D7940, func_2D7958, func_427C78.
   Needs an assembly body or a compiler route that selects lq/sq. ASM_FUNC candidate. */
