/* from src/levels/shared/L08_002F2D10.c: not written this wave. Retail 200 bytes, 3 place(s), first word at L08:003c66f0.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003c66f0  addiu       $sp, $sp, -0x50
 *   0x003c66f4  sd          $s1, 0x18($sp)
 *   0x003c66f8  sd          $s2, 0x20($sp)
 *   0x003c66fc  sd          $s3, 0x28($sp)
 *   0x003c6700  daddu       $s2, $a0, $zero
 *   0x003c6704  swc1        $f20, 0x40($sp)
 * Kept as data for the maintainers. */
