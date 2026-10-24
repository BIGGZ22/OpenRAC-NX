/* func_00100350 -- SCUS_976.23 0x00100350, 48 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00100350 (48 octets)
 *   Cadre 0x30, slots de 16 octets (sd/ld) : route sdk991111.
 *   Monte le bloc de requete de 32 octets sur la pile, appelle le
 *   syscall 0x40 (func_190D60 : addiu $v1,$zero,0x40 ; syscall 0),
 *   range le retour dans le global 0x00206300 et le rend.
 *   La forme compte : les deux ecritures de 1 sont emises dans l'ordre
 *   inverse des offsets (0x8 puis 0x4) par le planificateur ; le retail
 *   ecrit 0x4 puis 0x8, il faut donc ecrire d'abord le champ 0x8.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

struct BootRequest
{
    int f00;
    int f04;
    int f08;
    int f0c;
    int f10;
    int f14;
    int f18;
    int f1c;
};

extern int g_boot_00206300;
extern int func_190D60(struct BootRequest *request);

int func_00100350(void)
{
    struct BootRequest request;
    int result;

    request.f08 = 1;
    request.f04 = 1;
    result = func_190D60(&request);
    g_boot_00206300 = result;
    return result;
}
