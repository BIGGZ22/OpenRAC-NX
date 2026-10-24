/* LevelUpBolt_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendMode_LEVEL10_049210 -- LEVEL10.REL 0x049210, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   32 octets : la fonction relaie ses six parametres a
 *   Line_RegisterSystem et lui passe en 7e argument l'adresse d'un objet
 *   PAR_Group statique (0x49430), calculee par lui/addiu dans le delai du jal.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void Line_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendModePFPviPC9PAR_GroupPC29PAR_UpdateAndTesselateContext_i(
    int layer, int index, int zWriteMode, int colorKeyMode, int bloomMode,
    int blendMode, void *group);

/* objet PAR_Group statique du LevelUpBolt, a 0x49430 dans le module */
extern char g_levelUpBoltGroup[];

void LevelUpBolt_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendMode(
    int layer, int index, int zWriteMode, int colorKeyMode, int bloomMode,
    int blendMode)
{
    Line_RegisterSystem__F10DRAW_Layeri15DRAW_ZWriteMode17DRAW_ColorKeyMode14DRAW_BloomMode14DRAW_BlendModePFPviPC9PAR_GroupPC29PAR_UpdateAndTesselateContext_i(
        layer, index, zWriteMode, colorKeyMode, bloomMode, blendMode,
        g_levelUpBoltGroup);
}
