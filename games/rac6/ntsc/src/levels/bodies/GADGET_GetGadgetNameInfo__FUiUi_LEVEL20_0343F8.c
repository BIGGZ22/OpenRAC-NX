/* GADGET_GetGadgetNameInfo__FUiUi_LEVEL20_0343F8 -- LEVEL20.REL 0x0343F8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   L'information de nom du gadget est le bloc situe a 0x10 dans la definition
 *   du gadget au niveau demande (GADGET_GetDefAtLevel__FUiUi, 0x0334C8).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GADGET_DEF GADGET_DEF;

extern GADGET_DEF *GADGET_GetDefAtLevel__FUiUi(unsigned int gadgetId,
                                              unsigned int level);

GADGET_DEF *GADGET_GetGadgetNameInfo__FUiUi(unsigned int gadgetId,
                                            unsigned int level)
{
    GADGET_DEF *def = GADGET_GetDefAtLevel__FUiUi(gadgetId, level);

    return (GADGET_DEF *)((unsigned char *)def + 0x10);
}
