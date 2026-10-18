typedef struct { unsigned int w[4]; } DLIB_GIFTAG;
extern DLIB_GIFTAG g_cntTag; /* 0x01FD3360 */

void dlib_displist_base_offset_mscal__FP13DLIB_DISPLISTiii(int *list, int a, int b,
                                                          int c) {
    unsigned int *q;

    *(DLIB_GIFTAG *)list[2] = g_cntTag;
    q = (unsigned int *)list[1];
    q[0] = 0;
    q[1] = 0x14000000 | (short)c;
    q[2] = 0x03000000 | (short)a;
    q[3] = 0x02000000 | (short)b;
    list[1] += 0x10;
    *(unsigned short *)list[2] = (unsigned short)(((list[1] - list[2]) >> 4) - 1);
    list[2] = list[1];
    list[1] += 0x10;
}
