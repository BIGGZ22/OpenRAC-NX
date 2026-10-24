/* GADGET_GetGadgetNameInfo__FUiUi_LEVEL14_0344B0 -- LEVEL14.REL 0x0344B0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   32 octets.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern char *GADGET_GetDefAtLevel__FUiUi(unsigned int gadget, unsigned int level);

char *GADGET_GetGadgetNameInfo__FUiUi(unsigned int gadget, unsigned int level)
{
    return GADGET_GetDefAtLevel__FUiUi(gadget, level) + 0x10;
}
