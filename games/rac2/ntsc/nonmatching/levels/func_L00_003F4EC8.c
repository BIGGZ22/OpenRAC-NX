/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 724 bytes, 27 place(s), first word at L00:003f4ec8.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003f4ec8  addiu       $sp, $sp, -0x70
 *   0x003f4ecc  sd          $s0, 0x20($sp)
 *   0x003f4ed0  sd          $s1, 0x28($sp)
 *   0x003f4ed4  daddu       $s0, $a0, $zero
 *   0x003f4ed8  sd          $s2, 0x30($sp)
 *   0x003f4edc  addiu       $a0, $zero, 0xC04
 * Kept as data for the maintainers. */
