/* from src/levels/shared/L00_002E5918.c: eight bytes - `jr $ra` followed by the
 * COP1 word 0x460D6028 (opcode 0x11, format S, funct 0x28), which rabbitizer
 * prints as INVALID. The register fields are $f13 (ft), $f12 (fs) and $f0 (fd),
 * exactly the fields of the `mul.s $f0,$f12,$f13` this cc1 emits for
 * `return a0 * a1;` - so the candidate is the right size (8 bytes) with the same
 * two words but the funct 0x02 (mul.s) against retail's 0x28. The 0x28 funct in
 * the S format is outside the convert group (0x20 cvt.s.w, 0x24 cvt.s.d, 0x26
 * cvt.s.l) and this cc1 emits no COP1 word with it: add.s, sub.s, mul.s, div.s,
 * mov.s, c.eq.s, c.olt.s and the double-to-float conversion (which this cc1
 * lowers to a `jal` on a helper) were all tried. Handwritten candidate. */
float func_L00_002EE050(float a0, float a1) {
    return a0 * a1;
}
