/* func_00115C48 -- SCUS_976.23 0x00115C48, 32 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 0x00115C48 : fills a four-word object (id, default, value, extra).
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int g_boot_001FB9A0;

int *func_00115C48(int *self, int a1, int a2, int a3)
{
    self[0] = a1;
    self[1] = (int)&g_boot_001FB9A0;
    self[2] = a2;
    self[3] = a3;
    return self;
}
