typedef struct { float m[16]; } MATRIX4;
typedef struct { char pad[0x50]; MATRIX4 *matrix; } tGumState;

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

extern tDbDlist db_dlist;      /* 0x01F78B70 */
extern DLIB_GIFTAG g_cntTag;   /* 0x01FD3360 */
extern tGumState gGumState;    /* 0x01F77900 */
extern int g_CurLoadedScreenMatrix; /* 0x01EF56B4 */

extern void *GetMatrix__Q34scee3Psp3GumQ44scee3Psp2Gu11EMatrixType(unsigned int slot);
extern void SYSTEM_UploadMatrix__F15PS2_MATRIX_SLOTP7MATRIX4(int slot, MATRIX4 *m);

void SYSTEM_ThroughmodeMatrixIsDirty__Fv(void) {
    MATRIX4 *m;
    DLIB_DISPLIST *dl;
    unsigned int *q;

    if (g_CurLoadedScreenMatrix != 5) return;
    m = (MATRIX4 *)GetMatrix__Q34scee3Psp3GumQ44scee3Psp2Gu11EMatrixType(5);
    if (m == 0) return;
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
    SYSTEM_UploadMatrix__F15PS2_MATRIX_SLOTP7MATRIX4(4, m);
}
