/* LEVEL13_01E618 -- LEVEL13.REL 0x01E618, 64 bytes.
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

typedef struct GlobalVars {
    char pad[0xECC];
    int levelId;
} GlobalVars;

extern GlobalVars *pGV;
extern int G_levelId;

extern void GLOBALVARS_RestoreLevelData__FUi(unsigned int levelId);

void func_01E618(void)
{
    *(int *)((char *)pGV + 0xECC) = G_levelId;
    ((unsigned char *)pGV + 0x8000)[0x7755] = 0;
    GLOBALVARS_RestoreLevelData__FUi(G_levelId);
}
