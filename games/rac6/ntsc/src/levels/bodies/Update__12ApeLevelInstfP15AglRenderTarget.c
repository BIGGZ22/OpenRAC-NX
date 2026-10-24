/* Update__12ApeLevelInstfP15AglRenderTarget -- LEVEL10.REL 0x003750, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Mise a jour du niveau : si une cible de rendu est fournie et que la
 *   camera existe, ses parametres sont regles, puis le monde est mis a jour.
 *   Le parametre flottant passe en $f12 (non utilise ici) ; $a1 porte la
 *   cible de rendu.
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
typedef struct ApeCameraInstance ApeCameraInstance;
typedef struct AglRenderTarget AglRenderTarget;

extern void SetParamsToRenderTarget__17ApeCameraInstanceP15AglRenderTarget(
    ApeCameraInstance *camera, AglRenderTarget *target);
extern void Update__8ApeWorld(void *world);

struct ApeLevelInst {
    void *world;
    unsigned char unknown_004[0x04];
    ApeCameraInstance *camera;
};

void Update__12ApeLevelInstfP15AglRenderTarget(ApeLevelInst *self,
                                                float value,
                                                AglRenderTarget *target)
{
    (void)value;

    if (target != 0) {
        if (self->camera != 0) {
            SetParamsToRenderTarget__17ApeCameraInstanceP15AglRenderTarget(
                self->camera, target);
        }
    }
    Update__8ApeWorld(self->world);
}
