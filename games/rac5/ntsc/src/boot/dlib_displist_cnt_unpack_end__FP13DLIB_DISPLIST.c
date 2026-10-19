typedef struct {
    char pad0[4];
    unsigned char *cur;
    char pad1[8];
    unsigned char *unpack;
} DLIB_DISPLIST;

void dlib_displist_cnt_unpack_end__FP13DLIB_DISPLIST(DLIB_DISPLIST *displist) {
    unsigned char *start = displist->unpack;
    start[2] = (unsigned char)(((int)displist->cur - (int)start) >> 4);
    displist->unpack = 0;
}
