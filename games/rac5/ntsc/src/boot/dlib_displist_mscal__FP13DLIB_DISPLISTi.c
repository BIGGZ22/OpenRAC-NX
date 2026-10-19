/* Candidate only: the full-program build and compiler profile are not qualified. */
typedef struct {
    unsigned int w[4];
} DLIB_GIFTAG;

extern DLIB_GIFTAG g_cntTag;

void dlib_displist_mscal__FP13DLIB_DISPLISTi(int *list, int scale) {
    unsigned int *q;

    *(DLIB_GIFTAG *)list[2] = g_cntTag;
    q = (unsigned int *)list[1];
    q[3] = 0x14000000 | (short)scale;
    q[1] = 0;
    q[2] = 0;
    q[0] = 0;
    list[1] += 0x10;
    *(unsigned short *)list[2] = (unsigned short)(((list[1] - list[2]) >> 4) - 1);
    list[2] = list[1];
    list[1] += 0x10;
}
