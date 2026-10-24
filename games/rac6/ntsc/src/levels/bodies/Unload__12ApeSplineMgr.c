/* Unload__12ApeSplineMgr -- FE.REL 0x000A5F80, 28 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le dechargement se reduit a la remise a zero du gestionnaire de splines.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApeSplineMgr ApeSplineMgr;

extern void Reset__12ApeSplineMgr(ApeSplineMgr *self);

void Unload__12ApeSplineMgr(ApeSplineMgr *self)
{
    Reset__12ApeSplineMgr(self);
}
