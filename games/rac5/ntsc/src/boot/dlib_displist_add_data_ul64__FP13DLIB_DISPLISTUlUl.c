typedef struct {
    char pad0[4];
    unsigned long *cur;
} DLIB_DISPLIST;

void dlib_displist_add_data_ul64__FP13DLIB_DISPLISTUlUl(DLIB_DISPLIST *displist,
                                                        unsigned long first,
                                                        unsigned long second) {
    displist->cur[0] = first;
    displist->cur[1] = second;
    displist->cur = (unsigned long *)((unsigned char *)displist->cur + 0x10);
}
