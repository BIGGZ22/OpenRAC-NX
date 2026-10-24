/* WEAPON_GetCurrWeapAmmo__FPC6PLAYER -- LEVEL1.REL 0x0329A8, 32 bytes.
 *
 * Returns the ammunition of the weapon the player holds.
 *
 * Evidence:
 *   Munitions de l'arme courante du joueur (table des gadgets, element 0x74).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct WEAPON WEAPON;
typedef struct PLAYER PLAYER;

extern WEAPON *GADGET_GetData__FUi(unsigned int index);

struct WEAPON {
    unsigned char unknown_000[0x60];
    int ammo;
    unsigned char unknown_064[0x10];
};

struct PLAYER {
    unsigned char unknown_000[0x344];
    unsigned int weapon;
};

int WEAPON_GetCurrWeapAmmo__FPC6PLAYER(PLAYER *player)
{
    WEAPON *weapon = GADGET_GetData__FUi(player->weapon);
    return weapon->ammo;
}
