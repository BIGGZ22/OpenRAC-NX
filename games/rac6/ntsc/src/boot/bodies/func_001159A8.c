/* func_001159A8 -- SCUS_976.23 0x001159A8, 44 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 : func_001159A8 (44 octets)
 *   Rend le quatrieme argument si func_115900 reussit, zero sinon.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern int func_115900(void *a, int b, int c, int d);

int func_001159A8(void *a, int b, int c, int d)
{
    return func_115900(a, b, c, d) ? d : 0;
}
