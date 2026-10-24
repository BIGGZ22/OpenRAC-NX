/* Evaluate__15ApeAnimTreeNodef -- LEVEL13.REL 0x001E18F8, 144 bytes.
 *
 * Body translated from the retail module.
 *
 * Evidence:
 *   The node owns, at +0x0C, the list header (item table at +0x0C, item count
 *   at +0x10).  The count is read through the address of that header, and the
 *   item table is re-read from the node on every pass (the callee may change
 *   it).  Each item carries, at +0x24, a key stream whose phase (+0x38) is
 *   added to the item and whose procedure (+0x3C) is called with the time.
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
    char              m_pad00[0x38];
    short             m_phase;      /* 0x38 */
    ApeAnimKeyProc    m_proc;       /* 0x3C */
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

void Evaluate__15ApeAnimTreeNodef(ApeAnimTreeNode *self, float time)
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
