/* HUD_DrawIconSimple__FUiP17HUD_BoundingBox16Ui -- FE.REL 0x02EB80, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   HUD_DrawIconSimple__FUiP17HUD_BoundingBox16Ui @ FE.REL 0x02EB80 (64 bytes)
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AglTexture AglTexture;
typedef struct HUD_BoundingBox16 HUD_BoundingBox16;

extern unsigned int GetIconTextureID__14DrawObjManagerUi(unsigned int iconId);
extern void HUD_DrawTextureSimpleArea__FP10AglTextureP17HUD_BoundingBox16Ui(
    AglTexture *texture, HUD_BoundingBox16 *box, unsigned int color);

void HUD_DrawIconSimple(unsigned int iconId, HUD_BoundingBox16 *box, unsigned int color)
{
    unsigned int texture = GetIconTextureID__14DrawObjManagerUi(iconId);
    HUD_DrawTextureSimpleArea__FP10AglTextureP17HUD_BoundingBox16Ui(
        (AglTexture *)texture, box, color);
}
