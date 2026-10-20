/* func_L03_002ADC00 (156 bytes, 3 places) - MMI word, ASM_FUNC candidate.
 *
 * Readable frame: 0x70 bytes, $s1 = D_L03_00189E20, `lh 0x6BC($s1)` and `lw 0x660($s1)`
 * guard the body, then a stack vector at sp+0x10 is filled (sp+8 = 2.0f from
 * `lui $at,0x4000` + mtc1) and func_2E9488/func_2E9340-style calls follow with $a0 = sp
 * and $a2 = sp+0x10.
 * The wall is the single word 0x700014A9 at 0x002ADC4C, immediately before the
 * `sq $v0,0x0($sp)` at 0x002ADC50: opcode 0x1C (SPECIAL2), the "+0x7000_xxxx family" the
 * sixth wave recorded as the real MMI class - rabbitizer prints `.word # INVALID` and no C
 * text this cc1 accepts reaches it.  Everything around it is understood.
 */
