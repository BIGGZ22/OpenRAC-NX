/* Update__12ApeLevelInstfP15AglRenderTarget_LEVEL1_003750 -- LEVEL1.REL 0x003750, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   `self` doit survivre aux deux appels : il est donc recopie dans $s0.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApeLevelInst ApeLevelInst;
typedef struct AglRenderTarget AglRenderTarget;
typedef struct ApeCameraInstance ApeCameraInstance;
typedef struct ApeWorld ApeWorld;

struct ApeLevelInst {
    ApeWorld *m_world;
    unsigned int m_unknown_004;
    ApeCameraInstance *m_camera;
};

extern void SetParamsToRenderTarget__17ApeCameraInstanceP15AglRenderTarget(
    ApeCameraInstance *camera, AglRenderTarget *target);
extern void Update__8ApeWorld(ApeWorld *world);

void Update__12ApeLevelInstfP15AglRenderTarget(ApeLevelInst *self, float delta,
                                               AglRenderTarget *target)
{
    (void)delta;

    if (target != 0) {
        if (self->m_camera != 0) {
            SetParamsToRenderTarget__17ApeCameraInstanceP15AglRenderTarget(
                self->m_camera, target);
        }
    }
    Update__8ApeWorld(self->m_world);
}
