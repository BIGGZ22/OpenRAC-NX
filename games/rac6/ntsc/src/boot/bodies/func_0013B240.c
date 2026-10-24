/* func_0013B240 -- SCUS_976.23 0x0013B240, 72 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_0013B240 (72 octets)
 *   Appeleur de func_143A10 (une table de bits) : le premier argument de
 *   l'appelant est transmis tel quel, le second recoit l'adresse d'un
 *   entier local.  Si l'appel rend non nul, on met zero dans *a1 (si a1
 *   n'est pas nul) et on rend la main ; sinon on appelle func_143FF8 avec
 *   la valeur rendue dans le local et a1.
 *   La branche "bnel $s0,$zero" porte l'ecriture dans son slot de delay :
 *   sur le R5900 le slot d'une branche *likely* non prise est annule, ce
 *   qui donne "if (a1) *a1 = 0;".
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
extern int func_143FF8(int value, int *out);

void func_0013B240(unsigned int packed, int *a1)
{
    int local;

    if (func_143A10(packed, &local)) {
        if (a1) {
            *a1 = 0;
            return;
        }
        return;
    }
    func_143FF8(local, a1);
}
