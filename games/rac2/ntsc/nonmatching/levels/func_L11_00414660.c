/* from src/levels/shared/L11_002C5E80.c: not written this wave. Retail 288 bytes, 4 place(s), first word at L11:00414660.
 * Rule: MMI/LQ/SQ (op 0x1e).
 * No C attempt was made (wave-5 tour-2 discard rule).
 * Retail opens with:
 *   0x00414660  mtc1        $zero, $f8
 *   0x00414664  nop
 *   0x00414668  c.eq.s      $f12, $f8
 *   0x0041466c  nop
 *   0x00414670  bc1f        . + 4 + (0x5 << 2)
 *   0x00414674  nop
 * Kept as data for the maintainers. */
