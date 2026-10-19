typedef struct {
    char pad0[4];
    unsigned int *cur;
    char pad1[8];
    unsigned int *unpack;
} DLIB_DISPLIST;

void dlib_displist_cnt_unpack_begin__FP13DLIB_DISPLISTii(DLIB_DISPLIST *displist,
                                                         int high, int low) {
    unsigned int *p = displist->cur;
    *p++ = 0;
    *p++ = 0;
    *p++ = 0;
    *p = (unsigned int)high << 24 | (unsigned int)low;
    displist->unpack = p;
    displist->cur = (unsigned int *)((unsigned char *)displist->cur + 0x10);
}
