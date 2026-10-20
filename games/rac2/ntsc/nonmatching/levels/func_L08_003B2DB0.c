/* from src/levels/shared/L08_002F2D10.c: not written this wave. Retail 444 bytes, 3 place(s), first word at L08:003b2db0.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003b2db0  addiu       $sp, $sp, -0x60
 *   0x003b2db4  sd          $s1, 0x28($sp)
 *   0x003b2db8  sd          $s2, 0x30($sp)
 *   0x003b2dbc  daddu       $s1, $a0, $zero
 *   0x003b2dc0  sd          $s3, 0x38($sp)
 *   0x003b2dc4  addiu       $a0, $zero, 0x4EA
 * Kept as data for the maintainers. */
