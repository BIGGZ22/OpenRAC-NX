/* UPDATE_GameModeInit__Fv_LEVEL2_025CE0 -- LEVEL2.REL 0x025CE0, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef int GAME_MODE_LIST;

extern void UPDATE_SetGameMode__F14GAME_MODE_LISTUi(GAME_MODE_LIST mode,
                                                    unsigned int argument);

void UPDATE_GameModeInit__Fv(void)
{
    UPDATE_SetGameMode__F14GAME_MODE_LISTUi(1, 0);
}
