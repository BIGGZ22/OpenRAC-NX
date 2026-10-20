/* from src/levels/shared/L05_003D33E8.c: not written this wave. Retail 28 bytes, 4 place(s), first word at L05:00427968.
 * Rule: starts with addiu $sp,$sp,+0x40 (catalogue entry early).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x00427968  addiu       $sp, $sp, 0x40
 *   0x0042796c  nop
 *   0x00427970  addiu       $sp, $sp, 0x240
 *   0x00427974  nop
 *   0x00427978  addiu       $sp, $sp, 0x70
 *   0x0042797c  nop
 * The real body starts at 0x00427988 (padding before it).
 * Kept as data for the maintainers. */
