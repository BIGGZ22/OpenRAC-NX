/* func_00148DE8 -- SCUS_976.23 0x00148DE8, 60 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00148DE8 (60 octets)
 *   Comme func_00148D68, mais le second appel prend un flottant : $f12 est
 *   sauvegarde dans $f20 (callee-saved) avant le premier appel, puis relu.
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
extern void func_1490F0(int a, float b);

void func_00148DE8(int a, float b)
{
    int local;
    if (func_149098(a, &local))
        return;
    func_1490F0(local, b);
}
