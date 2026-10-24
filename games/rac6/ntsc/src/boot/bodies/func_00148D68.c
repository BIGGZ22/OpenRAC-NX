/* func_00148D68 -- SCUS_976.23 0x00148D68, 60 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00148D68 (60 octets)
 *   Si func_149098(self, &local) echoue (renvoie non nul), rend la main ;
 *   sinon appelle func_1490C8(local, arg2).
 *   func_149098 stocke self dans *local et renvoie 0.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2953:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int func_149098(int a, int *out);
extern void func_1490C8(int a, void *b);

void func_00148D68(int a, void *b)
{
    int local;
    if (func_149098(a, &local))
        return;
    func_1490C8(local, b);
}
