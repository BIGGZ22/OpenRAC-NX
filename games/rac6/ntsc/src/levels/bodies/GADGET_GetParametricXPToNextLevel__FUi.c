/* GADGET_GetParametricXPToNextLevel__FUi -- LEVEL19.REL 0x0337E8, 256 bytes.
 *
 * The parametric experience the next level gives for a gadget: the
 * value of the gadget over the value of its definition, the ratio
 * is bounded to one.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the contextual division route (the pinned chain plus a
 * patch that reserves the retail's instruction slots before a
 * division) compiles this source to the retail bytes of that
 * placement, byte for byte.  The route is measured, its provenance
 * is known, and its control keeps every other body of the level
 * modules exact.
 * The program-level gate is still pending, so the body is proved,
 * not promoted.
 */

typedef struct GADGET_DATA {
    unsigned char reserved[0x64];
    unsigned int value;
} GADGET_DATA;

typedef struct GADGET_DEF {
    unsigned char reserved[0x34];
    unsigned int value;
} GADGET_DEF;

extern int GADGET_CanGainLevel__FUi(unsigned int index);
extern GADGET_DATA *GADGET_GetData__FUi(unsigned int index);
extern GADGET_DEF *GADGET_GetDefAtLevel__FUiUi(unsigned int index,
                                               unsigned int level);
extern unsigned int GADGET_GetCurrentPowerLevel__FUi(unsigned int index);

float GADGET_GetParametricXPToNextLevel__FUi(unsigned int index)
{
    GADGET_DATA *data;
    GADGET_DEF *def;
    float numerator;
    float denominator;
    float result;

    if (GADGET_CanGainLevel__FUi(index) == 0) {
        return 0.0f;
    }
    data = GADGET_GetData__FUi(index);
    def = GADGET_GetDefAtLevel__FUiUi(index,
                                      GADGET_GetCurrentPowerLevel__FUi(index));
    {
        unsigned int raw = def->value;
        numerator = (float)data->value;
        denominator = (float)raw;
        result = numerator / (denominator + 0.000001f);
    }
    if (1.0f < result) {
        result = 1.0f;
    }
    return result;
}
