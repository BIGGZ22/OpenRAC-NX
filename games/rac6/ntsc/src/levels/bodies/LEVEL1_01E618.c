/* LEVEL1_01E618 -- LEVEL1.REL 0x01E618, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   64 octets.
 *   pGV est un pointeur opaque : le type de la structure des variables globales
 *   n'est pas connu ici, les deux champs sont donc atteints par leur decalage
 *   (0xECC et 0xF755).  C'est cette opacite qui oblige le compilateur a
 *   recharger pGV apres l'ecriture, comme le fait le retail.
 *   Le deplacement pGV + 0xF755 passe par le decompilement de constante de MIPS :
 *   0xF755 = 0x8000 (ori) + 0x7755 (decalage de l'octet).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef unsigned char u8;
typedef unsigned int u32;

typedef struct GlobalVars GlobalVars;

extern GlobalVars *pGV;
extern u32 G_levelId;

extern void GLOBALVARS_RestoreLevelData__FUi(u32 levelId);

void func_01E618(void)
{
    *(u32 *)((u8 *)pGV + 0xECC) = G_levelId;
    *(u8 *)((u8 *)pGV + 0xF755) = 0;
    GLOBALVARS_RestoreLevelData__FUi(G_levelId);
}
