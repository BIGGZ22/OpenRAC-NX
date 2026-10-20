/* func_L00_00427620 (200 bytes, 26 places) - CATALOGUE ENTRY DEFECT, not a C question.
 *
 * The row's first two words are `sb $a0,0x88($v1)` (0xA0640088) and `nop`, with $v1 never
 * set in the function: the real body starts 8 bytes later at 0x00427628 with
 * `addiu $sp,$sp,-0x70`.  The remaining 192 bytes are a complete, ordinary function -
 * $v0 = *($s0+0x70), a 0x30-subrecord at +0x30 and a 0x20-subrecord at +0x0 written with
 * 0x3D4CCCCD / 0x3E99999A / 0x3F800000 and half 1, two func_2EE0D0 calls on them, then
 * func_2EE970(sp, s0+0x40) and func_2EEBD8(s0, sp) before `sh $zero,0x7E($s0)`.
 * This is the padding class the seventh wave's brief lists: an entry that is 8 bytes early,
 * in 26 programs at once.  Data for the maintainers of the catalogue, not a near miss to
 * work; the address to record is 0x00427628 + 8 * (program index) form - all 26 places need
 * the same +8 shift.
 */
