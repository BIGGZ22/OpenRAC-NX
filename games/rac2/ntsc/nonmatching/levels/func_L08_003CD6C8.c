/* from src/levels/shared/L08_002F2D10.c: not written this wave. Retail 60 bytes, 3 place(s), first word at L08:003cd6c8.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003cd6c8  lui         $v1, 0x19
 *   0x003cd6cc  addiu       $sp, $sp, -0x10
 *   0x003cd6d0  addiu       $v1, $v1, -0x6160
 *   0x003cd6d4  sd          $ra, 0x0($sp)
 *   0x003cd6d8  .word       0x78620000                   # INVALID     $v1, $v0, 0x0 # 00000000 <InstrIdType: CPU_NORMAL>
 *   0x003cd6dc  .word       0x7CA20000                   # INVALID     $a1, $v0, 0x0 # 00000000 <InstrIdType: CPU_NORMAL>
 * Kept as data for the maintainers. */
