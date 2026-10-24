/* Evaluate__21ApeAnimProceduralNodef -- LEVEL13.REL 0x001E15F8, 236 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   The procedural node accumulates the time (+0x2C) and then walks two lists:
 *   the first one at +0x34 (items at +0x34, count at +0x38) whose items carry
 *   a key stream at +0x38 (phase +0x18, procedure +0x1C), the second one at
 *   +0x48 (items at +0x48, count at +0x4C) whose items carry a key stream at
 *   +0x20 (phase +0x10, procedure +0x14).
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApeAnimProceduralNode ApeAnimProceduralNode;
typedef struct ApeAnimNodeList ApeAnimNodeList;
typedef struct ApeAnimStreamA ApeAnimStreamA;
typedef struct ApeAnimStreamB ApeAnimStreamB;
typedef struct ApeAnimItemA ApeAnimItemA;
typedef struct ApeAnimItemB ApeAnimItemB;

typedef void (*ApeAnimKeyProc)(void *node, float time);

struct ApeAnimNodeList {
    void           *m_items;      /* 0x00 */
    unsigned short  m_count;      /* 0x04 */
};

struct ApeAnimStreamA {
    char              m_pad00[0x18];
    short             m_phase;    /* 0x18 */
    ApeAnimKeyProc    m_proc;     /* 0x1C */
};

struct ApeAnimStreamB {
    char              m_pad00[0x10];
    short             m_phase;    /* 0x10 */
    ApeAnimKeyProc    m_proc;     /* 0x14 */
};

struct ApeAnimItemA {
    char              m_pad00[0x38];
    ApeAnimStreamA   *m_stream;   /* 0x38 */
};

struct ApeAnimItemB {
    char              m_pad00[0x20];
    ApeAnimStreamB   *m_stream;   /* 0x20 */
};

struct ApeAnimProceduralNode {
    void                 *m_vtable;    /* 0x00 */
    void                 *m_unk04;
    void                 *m_unk08;
    void                 *m_unk0C;
    void                 *m_unk10;
    void                 *m_unk14;
    void                 *m_unk18;
    void                 *m_unk1C;
    void                 *m_unk20;
    void                 *m_unk24;
    void                 *m_unk28;
    float                 m_accum;     /* 0x2C */
    char                  m_pad30[0x04];
    ApeAnimNodeList       m_list1;     /* 0x34 */
    char                  m_pad3C[0x0C];
    ApeAnimNodeList       m_list2;     /* 0x48 */
};

void Evaluate__21ApeAnimProceduralNodef(ApeAnimProceduralNode *self, float time)
{
    ApeAnimNodeList *list1;
    ApeAnimNodeList *list2;
    unsigned int      i;

    self->m_accum = self->m_accum + time;

    for (i = 0; i < (list1 = &self->m_list1)->m_count; i++) {
        ApeAnimItemA *item =
            *((ApeAnimItemA **)((char *)self->m_list1.m_items + (i << 2)));
        ApeAnimStreamA *ks = item->m_stream;
        ks->m_proc((char *)item + ks->m_phase, time);
    }

    for (i = 0; i < (list2 = &self->m_list2)->m_count; i++) {
        ApeAnimItemB *item =
            *((ApeAnimItemB **)((char *)self->m_list2.m_items + (i << 2)));
        ApeAnimStreamB *ks = item->m_stream;
        ks->m_proc((char *)item + ks->m_phase, time);
    }
}
