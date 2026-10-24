/* Arena3FlippyFloor_Reset__FP4Moby -- LEVEL14.REL 0x000BCE68, 44 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Arena3FlippyFloor_Reset__FP4Moby - LEVEL14.REL 0x000BCE68, 44 bytes.
 *   The moby is put back in its flippy-floor shape, then reset.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct Moby Moby;

extern void Arena3FlippyFloor_ResetMobyToFlip__FP4Moby(Moby *moby);
extern void MOBY_Reset__FP4Moby(Moby *moby);

void Arena3FlippyFloor_Reset__FP4Moby(Moby *moby)
{
    Arena3FlippyFloor_ResetMobyToFlip__FP4Moby(moby);
    MOBY_Reset__FP4Moby(moby);
}
