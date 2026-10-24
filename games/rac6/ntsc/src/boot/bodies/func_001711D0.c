/* func_001711D0 -- SCUS_976.23 0x001711D0, 52 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_001711D0 (52 octets)
 *   Appelle func_174150 sur l'objet, pose le pointeur 0x00202C80 dans
 *   le champ +0x4C et rend l'objet (daddu $v0,$s0,$zero).
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2952+gnu-as:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern char g_boot_00202C80[];

struct BootObj
{
    char pad[0x4C];
    int field4c;
};

extern void func_174150(struct BootObj *object);

int func_001711D0(struct BootObj *object)
{
    int result;

    func_174150(object);
    object->field4c = (int)g_boot_00202C80;
    result = (int)object;
    return result;
}
