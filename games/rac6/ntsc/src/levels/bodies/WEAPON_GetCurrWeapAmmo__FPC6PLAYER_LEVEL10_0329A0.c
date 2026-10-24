/* WEAPON_GetCurrWeapAmmo__FPC6PLAYER_LEVEL10_0329A0 -- LEVEL10.REL 0x0329A0, 32 bytes.
 *
 * Returns the ammunition of the weapon the player holds.
 *
 * Evidence:
 *   retail: reads the weapon slot index at +0x344, asks func_033558 for the
 *   weapon record and returns the ammo word at +0x60 of that record.
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
typedef struct WEAPON WEAPON;

struct WEAPON {
    unsigned char pad[0x60];
    int ammo;
};

struct PLAYER {
    unsigned char pad[0x344];
    unsigned int weapon_slot;
};

WEAPON *func_033558(unsigned int index);

int WEAPON_GetCurrWeapAmmo__FPC6PLAYER(PLAYER *player)
{
    WEAPON *weapon = func_033558(player->weapon_slot);
    return weapon->ammo;
}
