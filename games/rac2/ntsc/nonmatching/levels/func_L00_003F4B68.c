/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 256 bytes, 27 place(s), first word at L00:003f4b68.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003f4b68  addiu       $sp, $sp, -0x50
 *   0x003f4b6c  sd          $s4, 0x20($sp)
 *   0x003f4b70  sd          $s2, 0x10($sp)
 *   0x003f4b74  daddu       $s4, $a0, $zero
 *   0x003f4b78  sd          $s3, 0x18($sp)
 *   0x003f4b7c  daddu       $s2, $a2, $zero
 * Kept as data for the maintainers. */
