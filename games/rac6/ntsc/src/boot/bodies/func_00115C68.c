/* func_00115C68 -- SCUS_976.23 0x00115C68, 80 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_00115C68 (80 octets)
 *   Initialisation paresseuse d'un objet global : le premier mot de
 *   l'objet (0x00297A00) est un drapeau ; s'il est nul, on appelle
 *   func_115D80 puis func_115968(&objet, &g_001FBA28, &g_00297A10).
 *   La fonction rend l'adresse de l'objet.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int g_boot_00297A00;
extern char g_boot_001FBA28[];
extern char g_boot_00297A10[];

extern void func_115D80(void);
extern void func_115968(void *object, void *a1, void *a2);

void *func_00115C68(void)
{
    if (g_boot_00297A00 == 0) {
        func_115D80();
        func_115968(&g_boot_00297A00, g_boot_001FBA28, g_boot_00297A10);
    }
    return &g_boot_00297A00;
}
