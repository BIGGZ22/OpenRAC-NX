/* __negdf2 -- FE.REL 0x00069DA8, 56 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   addiu $sp, $sp, -0x40
 *   sd    $a0, 0x20($sp)
 *   daddu $a1, $sp, $zero
 *   sd    $ra, 0x30($sp)
 *   jal   __unpack_d
 *   addiu $a0, $sp, 0x20
 *   lw    $v0, 0x4($sp)
 *   daddu $a0, $sp, $zero
 *   sltiu $v0, $v0, 0x1
 *   jal   __pack_d
 *   sw    $v0, 0x4($sp)
 *   ld    $ra, 0x30($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x40
 *   The soft floating point library: __unpack_d fills a 24 byte descriptor
 *   (kind, sign, exponent, fraction), the sign is flipped, __pack_d rebuilds
 *   the double.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct {
    int kind;
    int sign;
    int exponent;
    int pad;
    unsigned long long fraction;
} double_descriptor;

extern void __unpack_d(double *value, double_descriptor *out);
extern double __pack_d(double_descriptor *value);

double __negdf2(double value)
{
    double_descriptor descriptor;

    __unpack_d(&value, &descriptor);
    descriptor.sign = (descriptor.sign == 0);
    return __pack_d(&descriptor);
}
