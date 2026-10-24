/* LEVEL1_017B00 -- LEVEL1.REL 0x017B00, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail: calls MU_UnderdampedSpringSetup__FP11SpringCoefffff with the
 *   spring coefficient object and the three float constants
 *   0x427B53D1 (20*pi), 0x41A00000 (20.0f) and 0x3D088889 (1/30).
 *   The coefficient object is a module-local object the symbol table does
 *   not export; the harness resolves it from the retail relocation.
 *   The three constants are written as the exact decimal values the retail
 *   holds: the compiler's own folding of 20*pi is one ulp off.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct SpringCoeffs SpringCoeffs;

extern SpringCoeffs g_springCoefficients;

extern void MU_UnderdampedSpringSetup__FP11SpringCoefffff(SpringCoeffs *coefficients,
                                                          float frequency,
                                                          float damping,
                                                          float ratio);

void func_017B00(void)
{
    MU_UnderdampedSpringSetup__FP11SpringCoefffff(&g_springCoefficients,
                                                   62.831851959228516f,
                                                   20.0f,
                                                   0.03333333507180214f);
}
