/* SCRNARENA_Render__Fv -- LEVEL14.REL 0x000AFB08, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   SCRNARENA_Render__Fv - LEVEL14.REL 0x000AFB08, 32 bytes.
 *   The arena screen renders the pause menu of the module.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct PAUSEMENU tPAUSEMENU;

extern tPAUSEMENU D_3D4DC8;

extern void PAUSEMENU_Render__FP10tPAUSEMENU(tPAUSEMENU *menu);

void SCRNARENA_Render__Fv(void)
{
    PAUSEMENU_Render__FP10tPAUSEMENU(&D_3D4DC8);
}
