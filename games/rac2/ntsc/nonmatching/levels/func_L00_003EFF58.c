/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 328 bytes, 27 place(s), first word at L00:003eff58.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003eff58  addiu       $sp, $sp, -0x50
 *   0x003eff5c  lui         $v0, 0x1B
 *   0x003eff60  lw          $v0, 0x2BB4($v0)
 *   0x003eff64  sd          $s0, 0x10($sp)
 *   0x003eff68  daddu       $t0, $a1, $zero
 *   0x003eff6c  sd          $s1, 0x18($sp)
 * Kept as data for the maintainers. */
