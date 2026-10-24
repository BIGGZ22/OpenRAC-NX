/* func_00144E08 -- SCUS_976.23 0x00144E08, 32 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   SCUS_976.23 0x00144E08 : forwards to the boot routine at 0x001420F0.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (prodg2953:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

int func_1420F0(int, int, int, int);

int func_00144E08(int a, int b)
{
    return func_1420F0(a, b, 1, 0);
}
