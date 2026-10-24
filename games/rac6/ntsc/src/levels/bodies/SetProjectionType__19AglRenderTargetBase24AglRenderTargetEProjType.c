/* SetProjectionType__19AglRenderTargetBase24AglRenderTargetEProjType -- FE.REL 0x07B310, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   SetProjectionType__19AglRenderTargetBase24AglRenderTargetEProjType
 *   @ FE.REL 0x07B310 (8 bytes)
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct AglRenderTargetBase AglRenderTargetBase;

typedef enum AglRenderTargetEProjType {
    AglRenderTargetEProjType_Perspective = 0,
    AglRenderTargetEProjType_Orthographic = 1
} AglRenderTargetEProjType;

struct AglRenderTargetBase {
    unsigned char reserved[0x31];
    unsigned char projectionType;
};

void SetProjectionType(AglRenderTargetBase *self, AglRenderTargetEProjType type)
{
    self->projectionType = type;
}
