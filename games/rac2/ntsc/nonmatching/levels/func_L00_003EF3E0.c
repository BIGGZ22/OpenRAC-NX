/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 356 bytes, 27 place(s), first word at L00:003ef3e0.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003ef3e0  addiu       $sp, $sp, -0xA0
 *   0x003ef3e4  sd          $s0, 0x60($sp)
 *   0x003ef3e8  sd          $s2, 0x70($sp)
 *   0x003ef3ec  daddu       $s0, $a0, $zero
 *   0x003ef3f0  sd          $s4, 0x80($sp)
 *   0x003ef3f4  addiu       $a0, $zero, 0xCE5
 * Kept as data for the maintainers. */
