/* Turn the WAD header's stored offsets into absolute pointers.
   Two tables are walked: twelve entry chains at +0x04 (each node carrying a
   secondary offset at +0x84 rebased past a 0x10C0 byte block) and a 1024 entry
   table at +0xC0 whose nodes only chain through +0x6C. */

typedef struct WAD_EntryNode {
    unsigned int pad00[0x1C];      /* 0x00 */
    int next;                      /* 0x70 */
    unsigned int pad74[0x04];      /* 0x74 */
    int extra;                     /* 0x84 */
} WAD_EntryNode;

typedef struct WAD_TableNode {
    unsigned int pad00[0x1B];      /* 0x00 */
    int next;                      /* 0x6C */
} WAD_TableNode;

typedef struct WAD_HEADER {
    int unknown0;                  /* 0x00 */
    int entry[0xC];                /* 0x04 */
    unsigned int pad34[0x23];      /* 0x34 */
    int table[0x400];              /* 0xC0 */
} WAD_HEADER;

void WAD_OffsetsToPointers__FP10WAD_HEADER(WAD_HEADER *header) {
    WAD_EntryNode *node;
    WAD_TableNode *item;
    unsigned int i;
    unsigned int next;

    i = 0;
    do {
        next = i + 1;
        if (header->entry[i] != 0) {
            header->entry[i] += (int)header;
            node = (WAD_EntryNode *)header->entry[i];
            if (node->extra != 0) {
                node->extra = node->extra + (int)header + 0x10C0;
            }
            while (node != 0 && node->next != 0) {
                node->next = node->next + (int)header;
                node = (WAD_EntryNode *)node->next;
                if (node->extra != 0) {
                    node->extra = node->extra + (int)header + 0x10C0;
                }
            }
        }
        i = next;
    } while (i < 0xC);

    {
        int *table = header->table;
        unsigned int j = 0;
        unsigned int jnext;
        do {
            jnext = j + 1;
            if (table[j] != 0) {
                table[j] += (int)header;
                item = (WAD_TableNode *)table[j];
                while (item != 0 && item->next != 0) {
                    item->next = item->next + (int)header;
                    item = (WAD_TableNode *)item->next;
                }
            }
            j = jnext;
        } while (j < 0x400);
    }
}
