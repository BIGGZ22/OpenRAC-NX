/* func_00115900 -- SCUS_976.23 0x00115900, 64 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00115900 (64 octets)
 *   Deux objets sont-ils identiques ?  Meme pointeur : oui ; sinon on
 *   compare les ints qu'ils pointent et le resultat est vrai quand
 *   func_184C34 renvoie zero.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int func_184C34(int left, int right);

int func_00115900(int *left, int *right)
{
    int same = 0;

    if (right == left || func_184C34(*left, *right) == 0)
        same = 1;
    return same;
}
