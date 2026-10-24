/* func_0015BDC0 -- SCUS_976.23 0x0015BDC0, 32 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 0x0015BDC0 : stores a small code in the object, else returns 0x25.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (sdk991111:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

int func_0015BDC0(void *self, unsigned int code)
{
    if (code - 1 < 4) {
        *(int *)((char *)self + 0x6F8) = (int)code;
        return 0;
    }
    return 0x25;
}
