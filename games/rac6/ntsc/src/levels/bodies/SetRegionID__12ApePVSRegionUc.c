/* SetRegionID__12ApePVSRegionUc -- FE.REL 0x0AFDC8, 8 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   SetRegionID__12ApePVSRegionUc  (FE.REL 0x0AFDC8, 8 bytes)
 *   Retail:  jr $ra / sb $a1, 0x0($a0)
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApePVSRegion ApePVSRegion;

struct ApePVSRegion
{
    unsigned char regionID;
};

void SetRegionID__12ApePVSRegionUc(ApePVSRegion *self, unsigned char id)
{
    self->regionID = id;
}
