/* from src/levels/shared/L00_003EE100.c: not written this wave. Retail 468 bytes, 27 place(s), first word at L00:003f26c8.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x003f26c8  addiu       $sp, $sp, -0x240
 *   0x003f26cc  sd          $s5, 0x1F8($sp)
 *   0x003f26d0  swc1        $f22, 0x230($sp)
 *   0x003f26d4  daddu       $s5, $a1, $zero
 *   0x003f26d8  swc1        $f21, 0x228($sp)
 *   0x003f26dc  sd          $s0, 0x1D0($sp)
 * Kept as data for the maintainers. */
