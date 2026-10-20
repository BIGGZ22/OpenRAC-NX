/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 340 bytes, 27 place(s), first word at L00:003f5238.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003f5238  lui         $v0, 0x14
 *   0x003f523c  addiu       $sp, $sp, -0x70
 *   0x003f5240  lbu         $v1, -0x6A81($v0)
 *   0x003f5244  addiu       $a3, $zero, 0xE0
 *   0x003f5248  sd          $s0, 0x40($sp)
 *   0x003f524c  lui         $a2, 0x26
 * Kept as data for the maintainers. */
