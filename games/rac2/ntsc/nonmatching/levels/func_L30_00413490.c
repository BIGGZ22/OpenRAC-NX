/* 0 of 1 places match (not attempted): the row holds two MMI words,
   `.word 0x78620000` at +0x2C and `.word 0x7C820000` at +0x30 (lq/sq family,
   rabbitizer reports INVALID), so this cc1 cannot emit it. The rest is ordinary C:
   spawn func_316A50(0x118A), copy the owner into the new object's 0xB8, clear its
   0x30 byte, then clear three parallel 16-entry tables (0x260E30, 0x260EB0 word
   stores and 0x1B3D30 byte stores) in one loop and return the object. Not C. */
