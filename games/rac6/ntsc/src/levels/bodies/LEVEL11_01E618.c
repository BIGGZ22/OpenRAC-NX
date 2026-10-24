/* LEVEL11_01E618 -- LEVEL11.REL 0x01E618, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   la remise a zero d'un octet de la structure globale de jeu.
 *   pGV designe une structure de jeu : le niveau courant y est copie a
 *   l'offset 0xECC, un octet est mis a zero a l'offset 0xF755
 *   (0x8000 + 0x7755), puis GLOBALVARS_RestoreLevelData__FUi recoit
 *   G_levelId.
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

extern GlobalVars *D_0026C0B0;
extern int D_003CB684;

extern void GLOBALVARS_RestoreLevelData__FUi(unsigned int levelId);

void func_01E618(void)
{
    *(int *)((char *)D_0026C0B0 + 0xECC) = D_003CB684;
    ((unsigned char *)D_0026C0B0 + 0x8000)[0x7755] = 0;
    GLOBALVARS_RestoreLevelData__FUi(D_003CB684);
}
