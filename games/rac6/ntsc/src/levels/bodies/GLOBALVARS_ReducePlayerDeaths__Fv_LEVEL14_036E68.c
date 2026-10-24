/* GLOBALVARS_ReducePlayerDeaths__Fv_LEVEL14_036E68 -- LEVEL14.REL 0x036E68, 256 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   0x036EC0  <cas 0 : pGV->fEE5>
 *   0x036EE8  <cas 1 : pGV->fEE4>
 *   0x036F10  <cas 3 : pGV->fEE6>
 *   0x036F38  <cas 2 : pGV->fEE7>
 *   func_021B70(G_levelId) selectionne le niveau concerne ; selon ce
 *   niveau, un compteur (octet) de pGV est decremente de 4, ou remis a
 *   zero s'il est deja inferieur a 5.
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

extern int func_021B70(int levelId);

void GLOBALVARS_ReducePlayerDeaths__Fv(void)
{
    switch (func_021B70(G_levelId)) {
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
