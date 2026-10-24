/* func_00117698 -- SCUS_976.23 0x00117698, 64 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 0x00117698 : lazily initialises the global at 0x00206120 and returns it.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int g_boot_00206120;
extern const char g_boot_001FBAE0[];
extern void func_00115988(int *slot, const char *text);

int *func_00117698(void)
{
    if (g_boot_00206120 == 0) {
        func_00115988(&g_boot_00206120, g_boot_001FBAE0);
    }
    return &g_boot_00206120;
}
