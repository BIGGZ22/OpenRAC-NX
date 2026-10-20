/* from src/levels/shared/L11_002C5E80.c: not written this wave. Retail 512 bytes, 2 place(s), first word at L11:002c5e80.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x002c5e80  addiu       $sp, $sp, -0x80
 *   0x002c5e84  lui         $v1, 0x1B
 *   0x002c5e88  lw          $v1, 0x2DC4($v1)
 *   0x002c5e8c  sd          $s1, 0x38($sp)
 *   0x002c5e90  sd          $s2, 0x40($sp)
 *   0x002c5e94  daddu       $s1, $zero, $zero
 * Kept as data for the maintainers. */
