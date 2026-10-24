/* func_00171208 -- SCUS_976.23 0x00171208, 60 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00171208 (60 octets)
 *   Appelle func_172718 ; si le retour est nul, range le quatrieme
 *   argument dans le champ +0x50 et rend zero.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2952+gnu-as:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

struct BootObj
{
    char pad[0x4C];
    int field4c;
    int field50;
};

extern int func_172718(struct BootObj *object, int a1, int a2, int a3);

int func_00171208(struct BootObj *object, int a1, int a2, int a3)
{
    int result = func_172718(object, a1, a2, a3);

    if (result == 0) {
        object->field50 = a3;
        return 0;
    }
    return result;
}
