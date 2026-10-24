/* fpcmp -- FE.REL 0x0006A770, 76 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail body (76 bytes): the two floats are unpacked and compared by the
 *   soft float core, the comparison result is returned in $v0.
 *   addiu $sp, $sp, -0x50
 *   sd    $s0, 0x30($sp)
 *   addiu $a0, $sp, 0x20
 *   sd    $ra, 0x40($sp)
 *   daddu $a1, $sp, $zero
 *   swc1  $f12, 0x20($sp)
 *   jal   __unpack_f
 *   swc1  $f13, 0x24($sp)
 *   addiu $s0, $sp, 0x10
 *   addiu $a0, $sp, 0x24
 *   jal   __unpack_f
 *   daddu $a1, $s0, $zero
 *   daddu $a1, $s0, $zero
 *   jal   func_0006A658
 *   daddu $a0, $sp, $zero
 *   ld    $ra, 0x40($sp)
 *   ld    $s0, 0x30($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x50
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
    int fraction;
} float_descriptor;

extern void __unpack_f(float *value, float_descriptor *out);
extern int func_0006A658(float_descriptor *left, float_descriptor *right);

int fpcmp(float left, float right)
{
    float_descriptor first;
    float_descriptor second;

    __unpack_f(&left, &first);
    __unpack_f(&right, &second);
    return func_0006A658(&first, &second);
}
