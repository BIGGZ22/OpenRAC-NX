/* LEVEL1_0398E0 -- LEVEL1.REL 0x0398E0, 8 bytes.
 *
 * Returns -1.
 *
 * Evidence:
 *   La routine rend -1.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

int func_0398E0(void)
{
    return -1;
}
