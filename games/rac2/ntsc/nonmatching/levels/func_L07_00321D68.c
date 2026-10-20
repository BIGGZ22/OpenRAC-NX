/* Not C: the body carries raw 128-bit MMI words - `.word 0x7A420000` and `.word 0x7C820000`
   at +0x2C/+0x30 (opcode 0x1E/0x1F, `lq`/`sq`) and `.word 0x78620000` / `.word 0x7FA20000`
   at +0x38/+0x3C, around the `jal func_322878` stem call, the frame stores and the
   `jal func_3221C8` tail. No C spelling; ASM_FUNC candidate.
   The 132-byte row itself is sound (0x321D68 + 0x84 = 0x321DEC, ending `jr $ra;
   addiu $sp,$sp,0x40`), unlike the L05 rows in this lot. */
