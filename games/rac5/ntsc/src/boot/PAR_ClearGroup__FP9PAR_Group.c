typedef struct PAR_Node {
    struct PAR_Node *prev;   /* 0x00 */
    struct PAR_Node *next;   /* 0x04 */
} PAR_Node;

typedef struct PAR_Group {
    int w00;
    PAR_Node *list;          /* 0x04: circular particle list */
    struct PAR_Group *next;  /* 0x08 */
    struct PAR_Group *prev;  /* 0x0C */
    char pad10[2];
    unsigned char type;      /* 0x12 */
} PAR_Group;

extern PAR_Node *gPar;      /* 0x01F695A8 */
extern char gParPool1[];            /* 0x01F695B4 */
extern char gParPool2[];            /* 0x01F695C8 */
extern void PAR_FreePool1(void *pool, PAR_Node *n);
extern void PAR_FreePool2(void *pool, PAR_Node *n);

void PAR_ClearGroup__FP9PAR_Group(PAR_Group *g) {
    PAR_Node *n;
    int type;

    if (g->list == (PAR_Node *)g)
        return;
    g->next->prev = g->prev;
    g->prev->next = g->next;
    g->next = g;
    g->prev = g;
    type = g->type;
    while (g->list != (PAR_Node *)g) {
        n = g->list;
        n->prev->next = n->next;
        n->next->prev = n->prev;
        switch (type) {
        case 0:
            n->next = gPar;
            gPar = n;
            break;
        case 1:
            PAR_FreePool1(gParPool1, n);
            break;
        case 2:
            PAR_FreePool2(gParPool2, n);
            break;
        }
    }
}
