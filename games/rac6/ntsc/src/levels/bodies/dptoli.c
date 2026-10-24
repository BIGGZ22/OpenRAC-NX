/* dptoli -- FE.REL 0x00069C70, 148 bytes.
 *
 * Body translated from the retail module.
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
    unsigned int kind;
    int sign;
    int exponent;
    int pad;
    unsigned long long fraction;
} double_descriptor;

extern void __unpack_d(double *value, double_descriptor *out);

int dptoli(double value)
{
    double_descriptor descriptor;
    int result;

    __unpack_d(&value, &descriptor);
    if ((descriptor.kind ^ 2) == 0)
        return 0;
    if (descriptor.kind < 2)
        return 0;
    if ((descriptor.kind ^ 4) == 0)
        return descriptor.sign ? (int)0x80000000 : 0x7FFFFFFF;
    if (descriptor.exponent < 0)
        return 0;
    if (descriptor.exponent >= 0x1F)
        return descriptor.sign ? (int)0x80000000 : 0x7FFFFFFF;
    result = (int)(descriptor.fraction >> (60 - descriptor.exponent));
    if (descriptor.sign)
        result = -result;
    return result;

}
