/* GADGET_GetGadgetNameInfo__FUiUi -- LEVEL19.REL 0x0344F0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   retail: GADGET_GetDefAtLevel(a, b) + 0x10.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern unsigned char *GADGET_GetDefAtLevel__FUiUi(unsigned int a, unsigned int b);

unsigned char *GADGET_GetGadgetNameInfo__FUiUi(unsigned int a, unsigned int b)
{
    return GADGET_GetDefAtLevel__FUiUi(a, b) + 0x10;
}
