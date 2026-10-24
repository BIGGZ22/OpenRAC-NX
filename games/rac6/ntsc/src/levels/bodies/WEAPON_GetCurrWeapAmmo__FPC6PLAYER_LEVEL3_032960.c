/* WEAPON_GetCurrWeapAmmo__FPC6PLAYER_LEVEL3_032960 -- LEVEL3.REL 0x032960, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Munitions de l'arme courante du joueur (donnees du gadget, champ 0x60).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GADGET_DATA GADGET_DATA;
typedef struct PLAYER PLAYER;

extern GADGET_DATA *GADGET_GetData__FUi(unsigned int index);

struct GADGET_DATA {
    unsigned char unknown_000[0x60];
    int ammo;
};

struct PLAYER {
    unsigned char unknown_000[0x344];
    unsigned int currentWeapon;      /* 0x344 */
};

int WEAPON_GetCurrWeapAmmo__FPC6PLAYER(const PLAYER *player)
{
    GADGET_DATA *weapon = GADGET_GetData__FUi(player->currentWeapon);

    return weapon->ammo;
}
