/* func_00121FB8 -- SCUS_976.23 0x00121FB8, 8 bytes.
 *
 * Body translated from the boot executable.
 *
 * Evidence:
 *   Boot SCUS_976.23 0x00121FB8 (8 bytes).
 *   Returns 0.  The retail code is `jr $ra` with `daddu $v0, $zero, $zero`
 *   in its delay slot.
 *
 * Placement: virtual address in the boot executable; the other
 * placements of the same body are listed in config/boot/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

int func_00121FB8(void)
{
    return 0;
}
