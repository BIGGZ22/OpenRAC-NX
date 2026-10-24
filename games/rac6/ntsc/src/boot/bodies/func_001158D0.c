/* func_001158D0 -- SCUS_976.23 0x001158D0, 48 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_001158D0 (48 octets)
 *   Pose l'adresse 0x001FBA18 dans le champ +0x4 de l'objet, puis
 *   appelle func_115858 si le drapeau est impair.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void func_115858(void);
extern char g_boot_001FBA18[];

struct BootObj
{
    char pad[4];
    int field4;
};

void func_001158D0(struct BootObj *object, int flags)
{
    object->field4 = (int)g_boot_001FBA18;
    if (flags & 1)
        func_115858();
}
