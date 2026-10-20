/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 384 bytes, 27 place(s), first word at L00:003f5870.
 * Rule: MMI/LQ/SQ (op 0x1c).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003f5870  addiu       $sp, $sp, -0x80
 *   0x003f5874  .word       0x700014A9                   # INVALID     $zero, $zero, 0x14A9 # 00000000 <InstrIdType: CPU_NORMAL>
 *   0x003f5878  swc1        $f20, 0x70($sp)
 *   0x003f587c  .word       0x7FA20000                   # INVALID     $sp, $v0, 0x0 # 00000000 <InstrIdType: CPU_NORMAL>
 *   0x003f5880  lui         $at, 0x3F80
 *   0x003f5884  mtc1        $at, $f20
 * Kept as data for the maintainers. */
