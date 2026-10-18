/* dlib_displist_data_unpack_end__FP13DLIB_DISPLIST 0x01E942C8..0x01E94318 */
typedef struct {
    char pad0[4];
    unsigned char *cur;
    unsigned char *begin;
} DLIB_DISPLIST;

extern void dlib_displist_cnt_unpack_end__FP13DLIB_DISPLIST(DLIB_DISPLIST *displist);

void dlib_displist_data_unpack_end__FP13DLIB_DISPLIST(DLIB_DISPLIST *displist) {
    int count;

    dlib_displist_cnt_unpack_end__FP13DLIB_DISPLIST(displist);
    count = ((int)displist->cur - (int)displist->begin) >> 4;
    *(short *)displist->begin = (short)(count - 1);
    displist->begin = displist->cur;
    displist->cur = (unsigned char *)((int)displist->cur + 0x10);
}
