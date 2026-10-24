/* MineBouncer_TypeReset__FP8MobyType -- LEVEL14.REL 0x00102B08, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   MineBouncer_TypeReset__FP8MobyType - LEVEL14.REL 0x00102B08, 32 bytes.
 *   The mine bouncer resets its ball physics dispatcher.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct BALLPHYSICS_Dispatcher BALLPHYSICS_Dispatcher;
typedef struct MobyType MobyType;

extern BALLPHYSICS_Dispatcher D_3DD410;

extern void BALLPHYSICS_DispatcherReset__FP22BALLPHYSICS_Dispatcher(
    BALLPHYSICS_Dispatcher *dispatcher);

void MineBouncer_TypeReset__FP8MobyType(MobyType *type)
{
    (void)type;
    BALLPHYSICS_DispatcherReset__FP22BALLPHYSICS_Dispatcher(&D_3DD410);
}
