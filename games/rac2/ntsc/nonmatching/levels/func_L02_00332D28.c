/* from src/levels/shared/L02_002B5F50.c: not written this wave. Retail 456 bytes, 11 place(s), first word at L02:00332d28.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x00332d28  addiu       $sp, $sp, -0x60
 *   0x00332d2c  sd          $s3, 0x28($sp)
 *   0x00332d30  sd          $s0, 0x10($sp)
 *   0x00332d34  daddu       $s3, $a0, $zero
 *   0x00332d38  sd          $s2, 0x20($sp)
 *   0x00332d3c  addiu       $s0, $s3, 0x10
 * Kept as data for the maintainers. */
