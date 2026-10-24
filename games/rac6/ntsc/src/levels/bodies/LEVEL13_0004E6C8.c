/* LEVEL13_0004E6C8 -- LEVEL13.REL 0x0004E6C8, 168 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   unnamed body of LEVEL13.REL at 0x0004E6C8, 168 bytes.
 *   Body translated from the retail module.
 *   the body sits between DR_Clear__Fv (0x0004E6C0) and
 *   DISPLAY_ResetMatrixRenderStates__Fv (0x0004E7F8); the module calls it,
 *   which is why the bound is a call bound.  It has no exported name.
 *   Retail: addiu $sp,-0x40 / swc1 $f21,0x18 / swc1 $f23,0x28 / lui 0x3F00
 *   mtc1 $f21 / mov.s $f23,$f12 / sd $s0,0x0 / mul.s $f12,$f13,$f21
 *   swc1 $f25,0x38 / swc1 $f24,0x30 / daddu $s0,$a0 / sd $ra,0x8
 *   mov.s $f25,$f14 / swc1 $f22,0x20 / mov.s $f24,$f15 / jal tanf
 *   swc1 $f20,0x10 / lui 0x3F80 / mtc1 $f20 / nop / nop
 *   div.s $f22,$f20,$f0 / jal tanf / mul.s $f12,$f23,$f21 / nop / nop
 *   div.s $f12,$f20,$f0 / daddu $a0,$s0 / mov.s $f14,$f25
 *   mov.s $f15,$f24 / jal func_04E770 / mov.s $f13,$f22 / ld $s0,0x0
 *   ld $ra,0x8 / lwc1 $f25,0x38 / lwc1 $f24,0x30 / lwc1 $f23,0x28
 *   lwc1 $f22,0x20 / lwc1 $f21,0x18 / lwc1 $f20,0x10 / jr $ra
 *   addiu $sp,0x40
 *   Two tangents are taken on the half of two of the float arguments, the
 *   quotients by one are the two first float arguments of the call at the end,
 *   and the two last arguments travel unchanged.  The constant one crosses the
 *   calls in a callee saved float register, and the retail reserves the two
 *   instruction slots of the R5900 before each division.
 *   Placement: module-relative offset of the disc module.
 *   Proof: the gprfirst-divctx-sqrt route
 *   (gnu-ee-20261015-gprfirst + contextual division padding + square root
 *   slots, :-O2 -G0) compiles this source to the retail bytes of that
 *   placement, byte for byte.  The program-level gate is still pending, so the
 *   body is proved, not promoted.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gprfirst-divctx-sqrt:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern float tanf(float x);
extern void func_04E770(void *a0, float x, float y, float z, float w);

static void body_0004E6C8(void *a0, float p1, float p2, float p3, float p4)
{
    float x = 1.0f / tanf(p2 * 0.5f);
    float y = 1.0f / tanf(p1 * 0.5f);

    func_04E770(a0, y, x, p3, p4);
}
