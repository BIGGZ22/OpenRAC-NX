/* LevelUpBolt_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendMode_LEVEL1_049218 -- LEVEL1.REL 0x049218, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Retail: forwards its six arguments to Line_RegisterSystem and adds the
 *   module-local object at 0x49438 as the seventh argument.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct LevelUpBoltParams LevelUpBoltParams;

extern LevelUpBoltParams g_levelUpBoltParams;

extern void Line_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendModePFPviPC9PAR_GroupPC29PAR_UpdateAndTesselateContext_i(
    int layer,
    int index,
    int zwrite,
    int colorkey,
    int bloom,
    int blend,
    void *params);

void LevelUpBolt_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendMode(
    int layer,
    int index,
    int zwrite,
    int colorkey,
    int bloom,
    int blend)
{
    Line_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendModePFPviPC9PAR_GroupPC29PAR_UpdateAndTesselateContext_i(
        layer, index, zwrite, colorkey, bloom, blend, &g_levelUpBoltParams);
}
