/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 416 bytes, 27 place(s), first word at L00:003f6168.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003f6168  addiu       $sp, $sp, -0x40
 *   0x003f616c  sd          $s0, 0x10($sp)
 *   0x003f6170  sd          $s2, 0x20($sp)
 *   0x003f6174  sd          $s3, 0x28($sp)
 *   0x003f6178  addiu       $s2, $zero, 0x60
 *   0x003f617c  sd          $ra, 0x30($sp)
 * Kept as data for the maintainers. */
