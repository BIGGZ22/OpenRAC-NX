/* WEAPON_GetCurrWeapAmmo__FPC6PLAYER_LEVEL2_032870 -- LEVEL2.REL 0x032870, 32 bytes.
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

typedef struct PLAYER PLAYER;
typedef struct GADGET_DATA GADGET_DATA;

struct GADGET_DATA {
    unsigned char pad_000[0x60];
    unsigned int m_ammo;
};

struct PLAYER {
    unsigned char pad_000[0x344];
    unsigned int m_gadget;
};

extern GADGET_DATA *GADGET_GetData__FUi(unsigned int gadget);

unsigned int WEAPON_GetCurrWeapAmmo__FPC6PLAYER(PLAYER *player)
{
    return GADGET_GetData__FUi(player->m_gadget)->m_ammo;
}
