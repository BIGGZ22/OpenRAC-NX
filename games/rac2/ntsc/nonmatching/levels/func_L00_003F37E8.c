/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 668 bytes, 27 place(s), first word at L00:003f37e8.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003f37e8  addiu       $sp, $sp, -0x170
 *   0x003f37ec  sd          $s0, 0x110($sp)
 *   0x003f37f0  sd          $s2, 0x120($sp)
 *   0x003f37f4  daddu       $s0, $a0, $zero
 *   0x003f37f8  sd          $s3, 0x128($sp)
 *   0x003f37fc  addiu       $a0, $zero, 0x5
 * Kept as data for the maintainers. */
