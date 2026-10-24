/* EnemyController_TypeInit__FP8MobyType -- LEVEL14.REL 0x000F4108, 40 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   EnemyController_TypeInit__FP8MobyType - LEVEL14.REL 0x000F4108, 40 bytes.
 *   The type of the enemy moby (0xC0078B95) is stored in the module global
 *   at 0x2900557C.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct MobyType MobyType;

extern unsigned int D_29557C;

extern unsigned int MOBY_GetType__FUi(unsigned int type_id);

void EnemyController_TypeInit__FP8MobyType(MobyType *type)
{
    (void)type;
    D_29557C = MOBY_GetType__FUi(0xC0078B95);
}
