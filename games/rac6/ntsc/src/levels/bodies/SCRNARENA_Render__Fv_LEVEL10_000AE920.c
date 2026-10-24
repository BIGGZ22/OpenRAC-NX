/* SCRNARENA_Render__Fv_LEVEL10_000AE920 -- LEVEL10.REL 0x000AE920, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   L'ecran d'arene rend le menu de pause global (adresse 0x3AC2C8).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct tPAUSEMENU tPAUSEMENU;

extern tPAUSEMENU g_pausemenu;
extern void PAUSEMENU_Render__FP10tPAUSEMENU(tPAUSEMENU *menu);

void SCRNARENA_Render__Fv(void)
{
    PAUSEMENU_Render__FP10tPAUSEMENU(&g_pausemenu);
}
