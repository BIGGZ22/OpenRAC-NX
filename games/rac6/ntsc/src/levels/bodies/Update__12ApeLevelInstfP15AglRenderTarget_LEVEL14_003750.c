/* Update__12ApeLevelInstfP15AglRenderTarget_LEVEL14_003750 -- LEVEL14.REL 0x003750, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Mise a jour d'un niveau : si la cible de rendu (2e argument entier, la
 *   cible n'est pas utilisee par la suite) est nulle, le premier appel est
 *   saute ; sinon l'objet 0x8 de l'instance est.appele s'il n'est pas nul.
 *   func_230450 recoit toujours l'objet 0x0 de l'instance.
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

struct ApeLevelInst {
    void *object00;
    unsigned int pad04;
    void *object08;
};

extern void func_23E530(void *object);
extern void func_230450(void *object);

void Update__12ApeLevelInstfP15AglRenderTarget(ApeLevelInst *self, float f,
                                                AglRenderTarget *rt)
{
    if (rt) {
        void *object = self->object08;

        if (object)
            func_23E530(object);
    }

    func_230450(self->object00);
}
