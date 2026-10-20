/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 708 bytes, 27 place(s), first word at L00:003f5390.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003f5390  addiu       $sp, $sp, -0xC0
 *   0x003f5394  sd          $s2, 0x70($sp)
 *   0x003f5398  sd          $s3, 0x78($sp)
 *   0x003f539c  daddu       $s2, $a0, $zero
 *   0x003f53a0  sd          $s0, 0x60($sp)
 *   0x003f53a4  addiu       $s3, $sp, 0x20
 * Kept as data for the maintainers. */
