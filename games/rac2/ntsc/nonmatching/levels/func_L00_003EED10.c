/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 268 bytes, 27 place(s), first word at L00:003eed10.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003eed10  addiu       $sp, $sp, -0x40
 *   0x003eed14  sd          $s4, 0x20($sp)
 *   0x003eed18  sd          $s2, 0x10($sp)
 *   0x003eed1c  daddu       $s4, $a0, $zero
 *   0x003eed20  sd          $s3, 0x18($sp)
 *   0x003eed24  daddu       $s2, $a1, $zero
 * Kept as data for the maintainers. */
