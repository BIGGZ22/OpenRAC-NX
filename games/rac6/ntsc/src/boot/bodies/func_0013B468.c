/* func_0013B468 -- SCUS_976.23 0x0013B468, 176 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_0013B468 (176 octets)
 *   Appel de func_143A10 ; s'il rend non nul on rend la main, sinon on
 *   appelle func_1414B0 avec la valeur du local, la constante 1 et les
 *   huit flottants passes en parametre (qui doivent survivre a l'appel :
 *   ils sont copies dans $f20..$f27).
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2952+ps2eeas:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int func_143A10(unsigned int packed, int *out);
extern int func_1414B0(int value, int flag, float f1, float f2, float f3,
                       float f4, float f5, float f6, float f7, float f8);

void func_0013B468(unsigned int packed, float f1, float f2, float f3, float f4,
                   float f5, float f6, float f7, float f8)
{
    int local;

    if (func_143A10(packed, &local))
        return;
    func_1414B0(local, 1, f1, f2, f3, f4, f5, f6, f7, f8);
}
