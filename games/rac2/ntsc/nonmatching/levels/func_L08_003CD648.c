/* from src/levels/shared/L08_002F2D10.c: not written this wave. Retail 128 bytes, 3 place(s), first word at L08:003cd648.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003cd648  addiu       $sp, $sp, -0x20
 *   0x003cd64c  lui         $v0, 0x19
 *   0x003cd650  sd          $s1, 0x8($sp)
 *   0x003cd654  sd          $s0, 0x0($sp)
 *   0x003cd658  daddu       $s1, $a0, $zero
 *   0x003cd65c  sd          $ra, 0x10($sp)
 * Kept as data for the maintainers. */
