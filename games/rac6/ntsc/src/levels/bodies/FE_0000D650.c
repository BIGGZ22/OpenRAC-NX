/* FE_0000D650 -- FE.REL 0x0000D650, 168 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   func_00D650 -- FE.REL 0x00D650, 168 bytes.
 *   Body translated from the retail module.
 *   func_00D650  (FE.REL 0x00D650, 168 octets; the module has no exported
 *   name for it, the name of the address is used)
 *   Retail: addiu $sp,-0x40 / swc1 $f21,0x18 / swc1 $f23,0x28
 *   lui $at,0x3F00 / mtc1 $at,$f21 / mov.s $f23,$f12 / sd $s0,0x0
 *   mul.s $f12,$f13,$f21 / swc1 $f25,0x38 / swc1 $f24,0x30
 *   daddu $s0,$a0 / sd $ra,0x8 / mov.s $f25,$f14 / swc1 $f22,0x20
 *   mov.s $f24,$f15 / jal tanf / swc1 $f20,0x10
 *   lui $at,0x3F80 / mtc1 $at,$f20 / div.s $f22,$f20,$f0
 *   jal tanf / mul.s $f12,$f23,$f21 / div.s $f12,$f20,$f0
 *   daddu $a0,$s0 / mov.s $f14,$f25 / mov.s $f15,$f24
 *   jal func_00D6F8 / mov.s $f13,$f22
 *   The two half angles are converted into cotangents and handed to the
 *   helper that fills the plane; the 0.5f and the 1.0f constants live across
 *   the calls, so they are kept in the callee-saved registers $f21 and $f20.
 *   Placement: module-relative offset of the disc module; the other
 *   placements of the same body are listed in config/levels/bodies.tsv.
 *   Proof: the route gprfirst + contextual division padding
 *   (gnu-ee-20261015-gprfirst + divpad-contextuel:-O2 -G0) compiles this source
 *   to the retail bytes of that placement, byte for byte.  The program-level
 *   gate is still pending, so the body is proved, not promoted.
 *   The layout of the structures is the one this body reads; it is not
 *   confirmed by the data of the disc yet.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gprfirst-divctx:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern float tanf(float angle);

extern void func_00D6F8(float *out, float a, float b, float c, float d);

void func_00D650(float *out, float a, float b, float c, float d)
{
    float cotB = 1.0f / tanf(b * 0.5f);
    float cotA = 1.0f / tanf(a * 0.5f);

    func_00D6F8(out, cotA, cotB, c, d);
}
