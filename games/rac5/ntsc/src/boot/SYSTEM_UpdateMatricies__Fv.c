typedef struct {
    unsigned int w[4];
} DLIB_GIFTAG;

typedef struct {
    char pad0[4];
    unsigned int *cur;
    unsigned int *begin;
} DLIB_DISPLIST;

typedef struct {
    char pad[0x54];
    DLIB_DISPLIST *pDlist;
} tDbDlist;

extern tDbDlist db_dlist;                      /* 0x01F78B70 */
extern DLIB_GIFTAG g_cntTag;                   /* 0x01FD3360 */
extern int cur_MC;                             /* 0x01EF5710 */

extern void dlib_displist_mscal__FP13DLIB_DISPLISTi(DLIB_DISPLIST *list, int scale);

void SYSTEM_UpdateMatricies__Fv(void) {
    DLIB_DISPLIST *dl;
    unsigned int *q;

    if (cur_MC < 0) return;
    dl = db_dlist.pDlist;
    *(DLIB_GIFTAG *)dl->begin = g_cntTag;
    q = dl->cur;
    q[3] = 0x13000000;
    q[1] = 0;
    q[2] = 0;
    q[0] = 0;
    dl->cur += 4;
    *(unsigned short *)dl->begin = 1;
    dl->begin = dl->cur;
    dl->cur += 4;
    dlib_displist_mscal__FP13DLIB_DISPLISTi(db_dlist.pDlist, 0);
}
