/* ApePtrAdd__H1Zv_PX01Ui_PX01 -- FE.REL 0x0D7E60, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   ApePtrAdd__H1Zv_PX01Ui_PX01  (FE.REL 0x0D7E60, 8 bytes)
 *   Retail:  jr $ra / addu $v0, $a0, $a1
 *   Template pointer arithmetic: T* + unsigned int, T = void here.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

void *ApePtrAdd__H1Zv_PX01Ui_PX01(void *ptr, unsigned int offset)
{
    return (void *)((unsigned int)ptr + offset);
}
