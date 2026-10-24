/* WEAPON_AutoswitchToHypershot__FP6PLAYER -- LEVEL3.REL 0x0327F8, 32 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Si l'arme courante n'est pas deja l'hypershot (0x16), l'hypershot devient
 *   l'arme suivante (0x364) et l'arme courante est memorisee (0x358).
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

struct PLAYER {
    unsigned char unknown_000[0x344];
    unsigned int currentWeapon;      /* 0x344 */
    unsigned char unknown_348[0x10];
    unsigned int previousWeapon;      /* 0x358 */
    unsigned char unknown_35c[0x8];
    unsigned int nextWeapon;          /* 0x364 */
};

void WEAPON_AutoswitchToHypershot__FP6PLAYER(PLAYER *player)
{
    unsigned int current = player->currentWeapon;

    if (current != 0x16) {
        player->previousWeapon = current;
        player->nextWeapon = 0x16;
    }
}
