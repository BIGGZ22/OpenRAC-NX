/* GADGET_FillAmmo__FUi_LEVEL13_034080 -- LEVEL13.REL 0x034080, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   La quantite de munitions portee par le gadget est recopiée depuis la
 *   definition du gadget (champ 0x40) dans l'etat du gadget (champ 0x60).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GADGET_DATA GADGET_DATA;
typedef struct GADGET_DATA_DEF GADGET_DATA_DEF;

struct GADGET_DATA {
    unsigned char unknown_000[0x60];
    unsigned int ammo;
};

struct GADGET_DATA_DEF {
    unsigned char unknown_000[0x40];
    unsigned int ammo;
};

extern GADGET_DATA *GADGET_GetData__FUi(unsigned int gadgetId);
extern GADGET_DATA_DEF *GADGET_GetDataDef__FUi(unsigned int gadgetId);

void GADGET_FillAmmo__FUi(unsigned int gadgetId)
{
    GADGET_DATA *data = GADGET_GetData__FUi(gadgetId);

    data->ammo = GADGET_GetDataDef__FUi(gadgetId)->ammo;
}
