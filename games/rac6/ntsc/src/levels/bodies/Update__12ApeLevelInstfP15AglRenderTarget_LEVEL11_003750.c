/* Update__12ApeLevelInstfP15AglRenderTarget_LEVEL11_003750 -- LEVEL11.REL 0x003750, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Mise a jour : si la cible de rendu est presente, la camera (champ 0x8) est
 *   mise a jour puis le monde (champ 0x0) l'est aussi.
 *   (le float dt est passe dans $f12 : $a1 porte la cible de rendu.)
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
typedef struct ApeWorld ApeWorld;
typedef struct ApeCameraInstance ApeCameraInstance;
typedef struct AglRenderTarget AglRenderTarget;

struct ApeLevelInst {
    ApeWorld *world;
    unsigned int unknown_004;
    ApeCameraInstance *camera;
};

extern void SetParamsToRenderTarget__17ApeCameraInstanceP15AglRenderTarget(
    ApeCameraInstance *self, AglRenderTarget *target);
extern void Update__8ApeWorld(ApeWorld *self);

void Update__12ApeLevelInstfP15AglRenderTarget(ApeLevelInst *self, float dt,
                                               AglRenderTarget *target)
{
    (void)dt;

    if (target != 0) {
        if (self->camera != 0) {
            SetParamsToRenderTarget__17ApeCameraInstanceP15AglRenderTarget(
                self->camera, target);
        }
    }
    Update__8ApeWorld(self->world);
}
