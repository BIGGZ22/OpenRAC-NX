/* SanzHotSpotRenderCB__FPv_LEVEL11_000750B0 -- LEVEL11.REL 0x000750B0, 36 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le parametre du callback n'est pas utilise : le rendu du hotspot de
 *   Sanchez est suivi de sa remise a zero.
 *   Preuve : chaine gnu-ee-20261015:-O2 -G0 (egalement exacte sur
 *   gnu-ee-trial-20261015:-O2 -G0), comparaison octet pour octet au retail
 *   au placement vise.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void SanzHotSpotRender__Fv(void);
extern void SanzHotSpotReset__Fv(void);

void SanzHotSpotRenderCB__FPv(void *param)
{
    (void)param;

    SanzHotSpotRender__Fv();
    SanzHotSpotReset__Fv();
}
