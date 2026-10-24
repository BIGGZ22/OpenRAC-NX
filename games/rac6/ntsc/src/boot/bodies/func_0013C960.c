/* func_0013C960 -- SCUS_976.23 0x0013C960, 64 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_0013C960 (64 octets)
 *   Cadre 0x30 (slots de 16 octets, sq/lq).
 *   func_13DB70(self, &local) : si le retour n'est pas nul, on sort ;
 *   sinon func_13DF08(local, ptr, 1).
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2953:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int func_13DB70(void *self, int *out);
extern void func_13DF08(int value, void *ptr, int flag);

void func_0013C960(void *self, void *ptr)
{
    int local;

    if (func_13DB70(self, &local) != 0) {
        return;
    }
    func_13DF08(local, ptr, 1);
}
