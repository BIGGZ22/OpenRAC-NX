/* QuickEvaluate__15ApeAnimTreeNodef -- LEVEL13.REL 0x001E1988, 144 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   Same walk as Evaluate__15ApeAnimTreeNodef (0x001E18F8), but the key
 *   stream of an item holds its phase at +0x30 and its procedure at +0x34.
 *
 * Placement: module-relative offset of the disc module; the other
 * placements of the same body are listed in config/levels/bodies.tsv.
 * Proof: the pinned chain (gnu-ee-20261015-gprfirst) compiles this source to the
 * retail bytes of that placement, byte for byte.  The program-level
 * gate is still pending, so the body is proved, not promoted.
 * The layout of the structures is the one this body reads; it is not
 * confirmed by the data of the disc yet.
 */

typedef struct ApeAnimTreeNode ApeAnimTreeNode;
typedef struct ApeAnimKeyStream ApeAnimKeyStream;
typedef struct ApeAnimNodeList ApeAnimNodeList;

typedef void (*ApeAnimKeyProc)(void *node, float time);

struct ApeAnimKeyStream {
    char              m_pad00[0x30];
    short             m_phase;      /* 0x30 */
    ApeAnimKeyProc    m_proc;       /* 0x34 */
};

struct ApeAnimNodeList {
    ApeAnimTreeNode **m_items;      /* 0x0C */
    unsigned short    m_count;      /* 0x10 */
};

struct ApeAnimTreeNode {
    void                   *m_vtable; /* 0x00 */
    void                   *m_unk04;  /* 0x04 */
    void                   *m_unk08;  /* 0x08 */
    ApeAnimNodeList         m_list;   /* 0x0C */
    char                    m_pad14[0x10];
    ApeAnimKeyStream       *m_stream; /* 0x24 */
};

void QuickEvaluate__15ApeAnimTreeNodef(ApeAnimTreeNode *self, float time)
{
    ApeAnimNodeList *list;
    unsigned int      i;

    for (i = 0; i < (list = &self->m_list)->m_count; i++) {
        ApeAnimTreeNode *item =
            *((ApeAnimTreeNode **)((char *)self->m_list.m_items + (i << 2)));
        if (item) {
            ApeAnimKeyStream *ks = item->m_stream;
            ks->m_proc((char *)item + ks->m_phase, time);
        }
    }
}
