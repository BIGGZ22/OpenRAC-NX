/* LEVEL13_017B00 -- LEVEL13.REL 0x017B00, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct SpringCoeff {
    float stiffness;
    float damping;
    float mass;
    float a;
    float b;
    float c;
} SpringCoeff;

/* the shake spring coefficients live at 0x30B9E8 in the module (anonymous) */
extern SpringCoeff D_30B9E8;

extern void MU_UnderdampedSpringSetup__FP11SpringCoefffff(SpringCoeff *coeff,
                                                           float frequency,
                                                           float damping,
                                                           float mass);

void func_017B00(void)
{
    MU_UnderdampedSpringSetup__FP11SpringCoefffff(&D_30B9E8, 62.831852f, 20.0f,
                                                   0.033333335f);
}
