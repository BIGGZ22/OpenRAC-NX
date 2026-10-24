/* GLOBALVARS_ReducePlayerDeaths__Fv_LEVEL11_036EA8 -- LEVEL11.REL 0x036EA8, 256 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   ...
 *   0x036F00  <cas 0 : pGV->fEE5>
 *   0x036F28  <cas 1 : pGV->fEE4>
 *   0x036F50  <cas 3 : pGV->fEE6>
 *   0x036F78  <cas 2 : pGV->fEE7>
 *   Selon le type de jeu du niveau, un compteur (octet) de pGV est
 *   decremente de 4, ou remis a zero s'il vaut deja moins de 5.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct GV GV;

struct GV {
    unsigned char pad000[0xEE4];
    unsigned char fEE4;
    unsigned char fEE5;
    unsigned char fEE6;
    unsigned char fEE7;
};

extern GV *pGV;
extern int G_levelId;

extern int LEVEL_GetGameType__FUi(int levelId);

void GLOBALVARS_ReducePlayerDeaths__Fv(void)
{
    switch (LEVEL_GetGameType__FUi(G_levelId)) {
    case 0: {
        unsigned char v = pGV->fEE5;
        if (v < 5)
            pGV->fEE5 = 0;
        else
            pGV->fEE5 = v - 4;
    }
        break;
    case 1: {
        unsigned char v = pGV->fEE4;
        if (v < 5)
            pGV->fEE4 = 0;
        else
            pGV->fEE4 = v - 4;
    }
        break;
    case 3: {
        unsigned char v = pGV->fEE6;
        if (v < 5)
            pGV->fEE6 = 0;
        else
            pGV->fEE6 = v - 4;
    }
        break;
    case 2: {
        unsigned char v = pGV->fEE7;
        if (v < 5)
            pGV->fEE7 = 0;
        else
            pGV->fEE7 = v - 4;
    }
        break;
    }
}
