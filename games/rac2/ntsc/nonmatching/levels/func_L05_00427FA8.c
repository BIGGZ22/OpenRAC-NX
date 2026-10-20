/* from src/levels/shared/L05_003D33E8.c: not written this wave. Retail 28 bytes, 4 place(s), first word at L05:00427fa8.
 * Rule: starts with addiu $sp,$sp,+0x40 (catalogue entry early).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x00427fa8  addiu       $sp, $sp, 0x40
 *   0x00427fac  nop
 *   0x00427fb0  addiu       $sp, $sp, 0x240
 *   0x00427fb4  nop
 *   0x00427fb8  addiu       $sp, $sp, 0x70
 *   0x00427fbc  nop
 * The real body starts at 0x00427fc4 (padding before it).
 * Kept as data for the maintainers. */
