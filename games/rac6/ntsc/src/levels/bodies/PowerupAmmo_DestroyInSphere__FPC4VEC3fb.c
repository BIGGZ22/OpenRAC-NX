/* PowerupAmmo_DestroyInSphere__FPC4VEC3fb -- LEVEL11.REL 0x0010BC48, 264 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Detruit les mobies places dans une sphere (centre, rayon) : pour chaque
 *   element d'une liste globale dont le champ d'etat porte le motif 1, la
 *   distance au centre est comparee au carre du rayon ; si elle est plus
 *   petite, un coup de feu est emis (quand le drapeau est pose) et le moby
 *   est detruit.
 *   La distance est accumulee dans l'ordre x, y, z ; le carre du rayon vit
 *   dans $f20 (un flottant callee-saved) et survit aux deux appels.
 *   Forme : le tableau de base (`items`) et l'iterateur (`item`) sont deux
 *   variables distinctes — la chaine calcule alors la borne depuis $v1 et
 *   copie l'iterateur dans $s1, comme le retail ; la position est lue par un
 *   pointeur local (`pos`), ce qui derive l'IV +0x30.
 *   Preuve : chaine gnu-ee-20261015-gprfirst:-O2 -G0, comparaison octet pour
 *   octet au retail au placement vise (264/264, SHA-256 retail 6cd969b4...).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct VEC3 { float x, y, z; } VEC3;

typedef struct Item {
    char pad0[0x30];
    VEC3 pos;                        /* 0x30 : la position de l'element */
    char pad1[0x64 - 0x3C];
    unsigned int state;              /* 0x64 : l'etat, dont le motif 0xF */
    char pad2[0x80 - 0x68];
} Item;

typedef struct List {
    char pad0[0x0C];
    Item *items;
    char pad1[0x3C - 0x10];
    int count;
} List;

extern List *D_002854D8;
extern void FIRECRACKER_Emit__F27FIRECRACKER_SortedBuiltinIDPC4VEC3(
    int id, const VEC3 *pos);
extern void MOBY_DestroyMoby__FP4MobybN21(void *moby, int b1, int b2, int b3);

void PowerupAmmo_DestroyInSphere__FPC4VEC3fb(const VEC3 *center, float radius,
                                             int flag)
{
    List *list = D_002854D8;
    Item *items = list->items;
    Item *end = items + list->count;
    float r2 = radius * radius;
    float dx, dy, dz;
    Item *item;

    for (item = items; item != end; item++) {
        const VEC3 *pos = &item->pos;

        if ((item->state & 0xF) != 1) {
            continue;
        }
        dx = center->x - pos->x;
        dy = center->y - pos->y;
        dz = center->z - pos->z;
        if (dx * dx + dy * dy + dz * dz < r2) {
            if (flag) {
                FIRECRACKER_Emit__F27FIRECRACKER_SortedBuiltinIDPC4VEC3(
                    2, pos);
            }
            MOBY_DestroyMoby__FP4MobybN21(item, 1, 1, 1);
        }
    }
}
