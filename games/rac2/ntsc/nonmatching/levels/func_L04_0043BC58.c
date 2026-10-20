/* from src/levels/shared/L04_00417308.c: not written this wave. Retail 488 bytes, 4 place(s), first word at L04:0043bc58.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x0043bc58  addiu       $sp, $sp, -0x60
 *   0x0043bc5c  addiu       $v0, $zero, 0x1
 *   0x0043bc60  sd          $s0, 0x30($sp)
 *   0x0043bc64  sd          $s1, 0x38($sp)
 *   0x0043bc68  daddu       $s0, $a0, $zero
 *   0x0043bc6c  sd          $ra, 0x40($sp)
 * Kept as data for the maintainers. */
