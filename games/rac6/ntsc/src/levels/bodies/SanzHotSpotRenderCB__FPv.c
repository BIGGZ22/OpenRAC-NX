/* SanzHotSpotRenderCB__FPv -- LEVEL10.REL 0x000751F0, 36 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le rappel dessine les points chauds de Sanz puis les reinitialise.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void SanzHotSpotRender__Fv(void);
extern void SanzHotSpotReset__Fv(void);

void SanzHotSpotRenderCB__FPv(void *data)
{
    (void)data;
    SanzHotSpotRender__Fv();
    SanzHotSpotReset__Fv();
}
