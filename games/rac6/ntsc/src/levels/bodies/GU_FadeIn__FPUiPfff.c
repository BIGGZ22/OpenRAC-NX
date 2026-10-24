/* GU_FadeIn__FPUiPfff -- LEVEL10.REL 0x0001B698, 172 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   GU_FadeIn__FPUiPfff -- LEVEL10.REL 0x01B698, 172 bytes.
 *   Body translated from the retail module.
 *   GU_FadeIn__FPUiPfff  (LEVEL10.REL 0x01B698, 172 octets)
 *   Retail: addiu $sp,-0x30 / sd $s1,0x8 / sd $s2,0x10 / daddu $s1,$a1
 *   swc1 $f20,0x20 / daddu $s2,$a0 / sd $s0,0x0 / mov.s $f20,$f12
 *   sd $ra,0x18 / lwc1 $f0,0x0($s1) / add.s $f0,$f0,$f13
 *   c.lt.s $f20,$f0 / nop / bc1f .Lelse / swc1 $f0,0x0($s1)
 *   .Lelse: swc1 $f20,0x0($s1) / lwc1 $f0,0x0($s1)
 *   lui $v0,0xFF / lui $at,0x437E / ori $at,$at,0xE666
 *   mtc1 $at,$f12 / ori $v0,$v0,0xFFFF / lw $s0,0x0($s2)
 *   mul.s $f12,$f0,$f12 / jal fptoui / and $s0,$s0,$v0
 *   sll $v0,$v0,24 / or $s0,$s0,$v0 / sw $s0,0x0($s2)
 *   lwc1 $f0,0x0($s1) / c.eq.s $f0,$f20 / nop / bc1t .L1
 *   addiu $v1,$zero,1 / daddu $v1,$zero,$zero
 *   .L1: daddu $v0,$v1,$zero / ld $s0 / ld $s1 / ld $s2 / ld $ra
 *   lwc1 $f20,0x20($sp) / jr $ra / addiu $sp,$sp,0x30
 *   Sibling of GU_FadeOut__FPUiPfff (0x01B5E8): the fade value is stepped up
 *   by `step` instead of down, and clamped to `limit` from above.
 *   Placement: module-relative offset of the disc module.
 *   Proof: the route gprfirst (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this
 *   source to the retail bytes of that placement, byte for byte.  The program-
 *   level gate is still pending, so the body is proved, not promoted.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern unsigned int fptoui(float x);

unsigned long long GU_FadeIn__FPUiPfff(unsigned int *color, float *fade,
                                       float limit, float step)
{
    unsigned int u;
    int flag;
    float f;

    f = *fade + step;
    *fade = f;
    if (limit < f)
        *fade = limit;

    u = *color & 0x00FFFFFFu;
    u |= fptoui(*fade * 254.9f) << 24;
    *color = u;

    flag = (*fade == limit) ? 1 : 0;
    return flag;
}
