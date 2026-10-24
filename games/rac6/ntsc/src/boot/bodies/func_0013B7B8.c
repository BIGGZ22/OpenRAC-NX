/* func_0013B7B8 -- SCUS_976.23 0x0013B7B8, 72 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_0013B7B8 (72 octets)
 *   Meme forme que func_0013B240 : l'appel a func_143A10 rend un entier,
 *   le second argument de l'appelant est ecrit a zero s'il est non nul,
 *   sinon func_144438 est appele avec la valeur du local et cet argument.
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
extern int func_144438(int value, int *out);

void func_0013B7B8(unsigned int packed, int *a1)
{
    int local;

    if (func_143A10(packed, &local)) {
        if (a1) {
            *a1 = 0;
            return;
        }
        return;
    }
    func_144438(local, a1);
}
