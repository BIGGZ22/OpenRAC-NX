/* PAR_Reset__Fv_LEVEL10_05F188 -- LEVEL10.REL 0x05F188, 256 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Le corps parcourt le tableau d'elements situe a 0x32440C (0x2C octets
 *   par element, nombre d'elements a +0x2C00).  Pour chaque element la liste
 *   chainee qui y commence est reinitialisee : chaque noeud pointe sur lui-meme
 *   et le champ de tete est remplace par la sentinelle (element + 0x14).  Le
 *   bloc de slots a 0x3243D0 est ensuite rechaine (noeuds de 0x10 octets),
 *   puis les trois appels de fin (func_07CB28, func_07CB88, memset) sont
 *   effectues.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015:-O2 -G0) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ParNode ParNode;
typedef struct ParElem ParElem;
typedef struct ParHeader ParHeader;
typedef struct ParSlots ParSlots;

struct ParNode {
    ParNode *f00;
    ParNode *f04;
    ParNode *f08;
    ParNode *f0C;
};

struct ParElem {
    unsigned char pad00[0x14];
    ParNode *f14;
    ParNode *f18;
    ParNode *f1C;
    ParNode *f20;
    unsigned char pad24[0x08];
};

struct ParHeader {
    ParElem elems[256];
    int count;
};

struct ParSlots {
    ParNode *f00;
    ParNode *f04;
    int f08;
    void *f0C;
    unsigned char pad10[0x10];
    void *f20;
    unsigned char pad24[0x10];
    void *f34;
    unsigned int f38;
};

extern ParHeader D_0032440C;
extern ParSlots D_003243D0;

extern void func_07CB28(void *p);
extern void func_07CB88(void *p);
extern void *memset(void *s, int c, unsigned int n);

void PAR_Reset__Fv(void)
{
    ParElem *e = (ParElem *)&D_0032440C;
    ParElem *end = e + ((ParHeader *)e)->count;

    for (; e != end; e++) {
        ParNode *n = e->f20;
        while (n != (ParNode *)((char *)e + 0x14)) {
            ParNode *next = n->f0C;
            n->f08 = n;
            n->f0C = n;
            n->f04 = n;
            n->f00 = n;
            n = next;
        }
        e->f1C = (ParNode *)((char *)e + 0x14);
        e->f20 = (ParNode *)((char *)e + 0x14);
        e->f18 = (ParNode *)((char *)e + 0x14);
        e->f14 = (ParNode *)((char *)e + 0x14);
    }

    {
        ParSlots *a = &D_003243D0;
        ParNode *p = a->f04;
        int n = a->f08;
        ParNode *last = (ParNode *)((char *)p + n * 16 - 0x10);
        while (p != last) {
            p->f04 = (ParNode *)((char *)p + 0x10);
            p = (ParNode *)((char *)p + 0x10);
        }
        p->f04 = 0;
        D_003243D0.f00 = D_003243D0.f04;
        func_07CB28(&D_003243D0.f0C);
        func_07CB88(&D_003243D0.f20);
        memset(D_003243D0.f34, 0, D_003243D0.f38);
    }
}
