/* UPDATE_GameModeInit__Fv_LEVEL20_025D18 -- LEVEL20.REL 0x025D18, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Initialisation du mode de jeu : cas numero 1 du commutateur
 *   UPDATE_SetGameMode__F14GAME_MODE_LISTUi (0x025D60).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

extern void UPDATE_SetGameMode__F14GAME_MODE_LISTUi(int mode, void *arg);

void UPDATE_GameModeInit__Fv(void)
{
    UPDATE_SetGameMode__F14GAME_MODE_LISTUi(1, (void *)0);
}
