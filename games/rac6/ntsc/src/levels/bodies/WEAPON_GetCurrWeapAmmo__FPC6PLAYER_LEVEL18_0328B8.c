/* WEAPON_GetCurrWeapAmmo__FPC6PLAYER_LEVEL18_0328B8 -- LEVEL18.REL 0x0328B8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Munitions courantes : la donnee du gadget courant (champ 0x344 du joueur)
 *   est interrogee, son champ 0x60 est rendu tel quel.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct PLAYER PLAYER;
typedef struct GADGET_DATA GADGET_DATA;

struct PLAYER {
    unsigned char unknown_000[0x344];
    unsigned int current_weapon;
};

struct GADGET_DATA {
    unsigned char unknown_000[0x60];
    int ammo;
};

extern GADGET_DATA *GADGET_GetData__FUi(unsigned int id);

int WEAPON_GetCurrWeapAmmo__FPC6PLAYER(PLAYER *player)
{
    return GADGET_GetData__FUi(player->current_weapon)->ammo;
}
