/* GADGET_GetGadgetNameInfo__FUiUi_LEVEL1_0344F8 -- LEVEL1.REL 0x0344F8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le nom affichable est la chaine situee 0x10 octets plus loin que le
 *   descripteur de niveau rendu par GADGET_GetDefAtLevel.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern unsigned char *GADGET_GetDefAtLevel__FUiUi(unsigned int gadget,
                                                  unsigned int level);

unsigned char *GADGET_GetGadgetNameInfo__FUiUi(unsigned int gadget,
                                                unsigned int level)
{
    return GADGET_GetDefAtLevel__FUiUi(gadget, level) + 0x10;
}
