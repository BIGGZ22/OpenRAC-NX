/* from src/levels/shared/L00_002F4400.c: not written this wave. Retail 120 bytes, 27 place(s), first word at L00:002f74f0.
 * Rule: starts with addiu $sp,$sp,+0x10 (catalogue entry early).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x002f74f0  addiu       $sp, $sp, 0x10
 *   0x002f74f4  nop
 *   0x002f74f8  addiu       $sp, $sp, 0x40
 *   0x002f74fc  nop
 *   0x002f7500  addiu       $sp, $sp, 0x30
 *   0x002f7504  nop
 * The real body starts at 0x002f7530 (padding before it).
 * Kept as data for the maintainers. */
