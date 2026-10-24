/* fpsub -- FE.REL 0x0006A298, 100 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail body (100 bytes): like fpadd, but the sign of the second operand
 *   is flipped before the soft float addition.
 *   ... swc1 $f13, 0x34($sp) ...
 *   lw    $v0, 0x14($sp)        ; second.sign
 *   ...
 *   xori  $v0, $v0, 0x1
 *   jal   func_0006A008
 *   sw    $v0, 0x14($sp)        ; second.sign = flipped
 *   jal   __pack_f
 *   daddu $a0, $v0, $zero
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

float fpsub(float left, float right)
{
    float_descriptor first;
    float_descriptor second;
    float_descriptor difference;

    __unpack_f(&left, &first);
    __unpack_f(&right, &second);
    second.sign = second.sign ^ 1;
    return __pack_f(func_0006A008(&first, &second, &difference));
}
