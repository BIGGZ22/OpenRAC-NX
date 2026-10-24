/* TEXMGR_GetPtrToBitmapData__FUi -- FE.REL 0x01F3D8, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   TEXMGR_GetPtrToBitmapData__FUi @ FE.REL 0x01F3D8 (64 bytes)
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AgxTexture AgxTexture;

extern AgxTexture *GetTextureID__14DrawObjManagerUi(unsigned int id);
extern unsigned char *GetImagePtr__10AglTextureUc(AgxTexture *self, unsigned char mode);

unsigned char *TEXMGR_GetPtrToBitmapData(unsigned int id)
{
    AgxTexture *texture = GetTextureID__14DrawObjManagerUi(id);
    if (texture != 0)
        return GetImagePtr__10AglTextureUc(GetTextureID__14DrawObjManagerUi(id), 4);
    return 0;
}
