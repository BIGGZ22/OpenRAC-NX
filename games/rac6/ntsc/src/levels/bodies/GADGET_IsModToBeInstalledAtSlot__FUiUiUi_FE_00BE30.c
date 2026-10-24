/* GADGET_IsModToBeInstalledAtSlot__FUiUiUi_FE_00BE30 -- FE.REL 0x00BE30, 128 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le gadget (identifiant "gadgetId") porte une table de 16 identifiants de
 *   modules a l'offset 0x30. La fonction renvoie vrai si l'entree "slot" de
 *   cette table porte l'identifiant de module "modId" ; une table vide renvoie
 *   faux. Deux appels distincts a GADGET_GetData__FUi (le second dans la
 *   branche).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GADGET_DATA {
    unsigned char unknown_000[0x30];
    unsigned int mods[16];
} GADGET_DATA;

extern GADGET_DATA *GADGET_GetData__FUi(unsigned int gadgetId);
extern unsigned int GADGET_GetModDef__FUi(unsigned int modId);

unsigned long long GADGET_IsModToBeInstalledAtSlot__FUiUiUi(unsigned int gadgetId,
                                                             unsigned int modId,
                                                             unsigned int slot)
{
    unsigned long long result = 0;

    if (GADGET_GetData__FUi(gadgetId)->mods[slot]) {
        unsigned int *p2 = GADGET_GetData__FUi(gadgetId)->mods;
        unsigned int *m2 = &p2[slot];
        int t = (*m2 == GADGET_GetModDef__FUi(modId));

        result = t;
    }

    return result;
}
