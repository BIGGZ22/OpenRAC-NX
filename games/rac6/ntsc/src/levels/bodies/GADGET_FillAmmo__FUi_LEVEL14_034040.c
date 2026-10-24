/* GADGET_FillAmmo__FUi_LEVEL14_034040 -- LEVEL14.REL 0x034040, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   64 octets.
 *   `data` et `gadgetId` sont vivants pendant le second appel : ils vivent
 *   donc dans les registres callee-saved $s1 et $s0.
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
typedef struct GADGET_DATA GADGET_DATA;

extern GADGET_DATA *GADGET_GetData__FUi(unsigned int gadgetId);
extern GADGET_DEF *GADGET_GetDataDef__FUi(unsigned int gadgetId);

struct GADGET_DEF {
    unsigned char reserved_000[0x40];
    unsigned int field_040;
};

struct GADGET_DATA {
    unsigned char reserved_000[0x60];
    unsigned int field_060;
};

void GADGET_FillAmmo__FUi(unsigned int gadgetId)
{
    GADGET_DATA *data = GADGET_GetData__FUi(gadgetId);
    GADGET_DEF *def = GADGET_GetDataDef__FUi(gadgetId);

    data->field_060 = def->field_040;
}
