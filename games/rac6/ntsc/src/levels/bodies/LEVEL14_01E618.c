/* LEVEL14_01E618 -- LEVEL14.REL 0x01E618, 64 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le pointeur global pGV designe une structure de jeu ; le niveau courant
 *   G_levelId y est copie a l'offset 0xECC, un octet est mis a zero a
 *   l'offset 0xF755, puis func_037758 recoit G_levelId.
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
    unsigned char pad000[0xECC];
    int level;
    unsigned char padF00[0x8000 - 0xECC - 4];
    unsigned char pad8000[0x7755];
    unsigned char flag;
};

extern void *pGV;
extern int G_levelId;

extern void func_037758(int levelId);

void func_01E618(void)
{
    *(int *)((char *)pGV + 0xECC) = G_levelId;
    ((GV *)pGV)->flag = 0;
    func_037758(G_levelId);
}
