/* from src/levels/shared/L02_002B5F50.c: not written this wave. Retail 232 bytes, 12 place(s), first word at L02:00333218.
 * Rule: MMI/LQ/SQ (op 0x1c).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x00333218  addiu       $sp, $sp, -0x60
 *   0x0033321c  .word       0x700014A9                   # INVALID     $zero, $zero, 0x14A9 # 00000000 <InstrIdType: CPU_NORMAL>
 *   0x00333220  .word       0x7FA20020                   # INVALID     $sp, $v0, 0x20 # 00000000 <InstrIdType: CPU_NORMAL>
 *   0x00333224  sd          $s0, 0x30($sp)
 *   0x00333228  sd          $s1, 0x38($sp)
 *   0x0033322c  daddu       $s0, $a0, $zero
 * Kept as data for the maintainers. */
