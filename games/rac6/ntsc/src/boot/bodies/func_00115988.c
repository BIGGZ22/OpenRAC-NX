/* func_00115988 -- SCUS_976.23 0x00115988, 28 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00115988 (28 octets)
 *   Initialise les deux premiers champs de l'objet (l'un d'eux est
 *   l'adresse 0x001FB9D0) ; l'objet nul est ignore.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern char g_boot_001FB9D0[];

struct BootObj
{
    int field00;
    int field04;
};

void func_00115988(struct BootObj *object, int a1)
{
    if (object) {
        object->field00 = a1;
        object->field04 = (int)g_boot_001FB9D0;
    }
}
