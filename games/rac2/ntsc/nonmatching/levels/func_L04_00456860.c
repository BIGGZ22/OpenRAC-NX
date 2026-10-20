/* from src/levels/shared/L04_00417308.c: not written this wave. Retail 396 bytes, 2 place(s), first word at L04:00456860.
 * Rule: MMI/LQ/SQ (op 0x1c).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x00456860  addiu       $sp, $sp, -0x90
 *   0x00456864  .word       0x700014A9                   # INVALID     $zero, $zero, 0x14A9 # 00000000 <InstrIdType: CPU_NORMAL>
 *   0x00456868  sd          $s2, 0x30($sp)
 *   0x0045686c  swc1        $f26, 0x70($sp)
 *   0x00456870  daddu       $s2, $a0, $zero
 *   0x00456874  swc1        $f25, 0x68($sp)
 * Kept as data for the maintainers. */
