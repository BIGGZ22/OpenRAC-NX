/* UPDATE_GameModeInit__Fv_LEVEL3_025DD0 -- LEVEL3.REL 0x025DD0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Initialisation du mode de jeu : mode numero 1 du commutateur de mode de jeu.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef int GAME_MODE_LIST;   /* enum GAME_MODE_LIST du retail */

extern void UPDATE_SetGameMode__F14GAME_MODE_LISTUi(GAME_MODE_LIST mode,
                                                     unsigned int arg);

void UPDATE_GameModeInit__Fv(void)
{
    UPDATE_SetGameMode__F14GAME_MODE_LISTUi(1, 0);
}
