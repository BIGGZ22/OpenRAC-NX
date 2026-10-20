/* from src/levels/shared/L02_002B5F50.c: not written this wave. Retail 12 bytes, 12 place(s), first word at L02:0032f008.
 * Rule: starts with addiu $sp,$sp,+0xc0 (catalogue entry early).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x0032f008  addiu       $sp, $sp, 0xC0
 *   0x0032f00c  nop
 *   0x0032f010  addiu       $sp, $sp, 0xC0
 *   0x0032f014  nop
 *   0x0032f018  addiu       $sp, $sp, -0x10
 *   0x0032f01c  addiu       $v0, $zero, 0xFF
 * The real body starts at 0x0032f018 (padding before it).
 * Kept as data for the maintainers. */
