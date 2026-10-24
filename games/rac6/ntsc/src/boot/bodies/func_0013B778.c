/* func_0013B778 -- SCUS_976.23 0x0013B778, 60 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_0013B778 (60 octets)
 *   Appel de func_143A10 ; s'il rend non nul on rend la main, sinon on
 *   appelle func_141B40 avec la valeur du local et le second argument.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2952+gnu-as:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int func_143A10(unsigned int packed, int *out);
extern int func_141B40(int value, int *out);

void func_0013B778(unsigned int packed, int *a1)
{
    int local;

    if (func_143A10(packed, &local))
        return;
    func_141B40(local, a1);
}
