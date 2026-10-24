/* func_001711A0 -- SCUS_976.23 0x001711A0, 48 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_001711A0 (48 octets)
 *   Pose le pointeur 0x00202E70 dans le champ +0x4C de l'objet, puis
 *   appelle func_115858 si le drapeau est impair.
 *   La valeur stockee est une ADRESSE (lui + addiu), pas une constante :
 *   le retail utilise addiu, ce qui est la forme d'une adresse symbolique.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2952+gnu-as:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void func_115858(void);
extern char g_boot_00202E70[];

struct BootObj
{
    char pad[0x4C];
    int field4c;
};

void func_001711A0(struct BootObj *object, int flags)
{
    object->field4c = (int)g_boot_00202E70;
    if (flags & 1)
        func_115858();
}
