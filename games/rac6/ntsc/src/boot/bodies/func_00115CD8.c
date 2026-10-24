/* func_00115CD8 -- SCUS_976.23 0x00115CD8, 28 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00115CD8 (28 octets)
 *   Initialise l'objet (champ +4 = adresse 0x001FB9B8) et le rend.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern char g_boot_001FB9B8[];

struct BootObj
{
    int field00;
    int field04;
    int field08;
};

struct BootObj *func_00115CD8(struct BootObj *object, int a1, int a2)
{
    object->field00 = a1;
    object->field04 = (int)g_boot_001FB9B8;
    object->field08 = a2;
    return object;
}
