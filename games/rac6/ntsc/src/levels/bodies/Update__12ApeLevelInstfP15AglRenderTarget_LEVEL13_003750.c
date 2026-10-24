/* Update__12ApeLevelInstfP15AglRenderTarget_LEVEL13_003750 -- LEVEL13.REL 0x003750, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApeWorld ApeWorld;
typedef struct ApeCameraInstance ApeCameraInstance;
typedef struct AglRenderTarget AglRenderTarget;

typedef struct ApeLevelInst {
    ApeWorld *world;
    int level;
    ApeCameraInstance *camera;
} ApeLevelInst;

extern void SetParamsToRenderTarget__17ApeCameraInstanceP15AglRenderTarget(
    ApeCameraInstance *self, AglRenderTarget *target);
extern void Update__8ApeWorld(ApeWorld *self);

void Update__12ApeLevelInstfP15AglRenderTarget(ApeLevelInst *self, float f,
                                              AglRenderTarget *target)
{
    (void)f;
    if (target) {
        if (self->camera)
            SetParamsToRenderTarget__17ApeCameraInstanceP15AglRenderTarget(
                self->camera, target);
    }
    Update__8ApeWorld(self->world);
}
