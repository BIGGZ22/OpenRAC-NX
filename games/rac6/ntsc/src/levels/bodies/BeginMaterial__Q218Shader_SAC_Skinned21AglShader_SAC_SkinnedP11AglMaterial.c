/* BeginMaterial__Q218Shader_SAC_Skinned21AglShader_SAC_SkinnedP11AglMaterial -- FE.REL 0x000C02C0, 40 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le etat du materiau est pose avec le microcode 7, puis le drapeau global
 *   "materiau begun" (octet en 0x251F58) est mis a 1.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AglMaterial AglMaterial;
typedef struct Shader_SAC_Skinned Shader_SAC_Skinned;

extern char D_251F58;

extern void SetMaterialState__9AglShaderP11AglMaterial18AglPs2VU1MicroCode(
    Shader_SAC_Skinned *self, AglMaterial *material, int microCode);

void BeginMaterial__Q218Shader_SAC_Skinned21AglShader_SAC_SkinnedP11AglMaterial(
    Shader_SAC_Skinned *self, AglMaterial *material)
{
    SetMaterialState__9AglShaderP11AglMaterial18AglPs2VU1MicroCode(self, material,
                                                                    7);
    D_251F58 = 1;
}
