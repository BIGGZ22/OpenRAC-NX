/* ANIM_SetInstanceAnimation__FP13GDE_AnimationP13ANIM_Instance -- LEVEL1.REL 0x04A258, 8 bytes.
 *
 * Sets the animation of an animation instance.
 *
 * Evidence:
 *   (8 octets)
 *   L'instance recoit l'animation dans son premier champ.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GDE_Animation GDE_Animation;
typedef struct ANIM_Instance ANIM_Instance;

struct ANIM_Instance {
    GDE_Animation *animation;
};

void ANIM_SetInstanceAnimation__FP13GDE_AnimationP13ANIM_Instance(
    GDE_Animation *animation, ANIM_Instance *instance)
{
    instance->animation = animation;
}
