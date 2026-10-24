/* fpadd -- FE.REL 0x0006A240, 88 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail body (88 bytes): the two floats are unpacked into 16 byte
 *   descriptors, added by the soft float core and packed back.
 *   addiu $sp, $sp, -0x60
 *   sd    $s0, 0x40($sp)
 *   addiu $a0, $sp, 0x30
 *   sd    $ra, 0x50($sp)
 *   daddu $a1, $sp, $zero
 *   swc1  $f12, 0x30($sp)
 *   jal   __unpack_f
 *   swc1  $f13, 0x34($sp)
 *   addiu $s0, $sp, 0x10
 *   addiu $a0, $sp, 0x34
 *   jal   __unpack_f
 *   daddu $a1, $s0, $zero
 *   daddu $a1, $s0, $zero
 *   addiu $a2, $sp, 0x20
 *   jal   func_0006A008
 *   daddu $a0, $sp, $zero
 *   jal   __pack_f
 *   daddu $a0, $v0, $zero
 *   ld    $ra, 0x50($sp)
 *   ld    $s0, 0x40($sp)
 *   jr    $ra
 *   addiu $sp, $sp, 0x60
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
extern float __pack_f(float_descriptor *value);
extern float_descriptor *func_0006A008(float_descriptor *left,
                                       float_descriptor *right,
                                       float_descriptor *result);

float fpadd(float left, float right)
{
    float_descriptor first;
    float_descriptor second;
    float_descriptor sum;

    __unpack_f(&left, &first);
    __unpack_f(&right, &second);
    return __pack_f(func_0006A008(&first, &second, &sum));
}
