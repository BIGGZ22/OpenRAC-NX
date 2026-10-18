typedef unsigned int QW __attribute__((mode(TI)));

typedef struct {
    char pad0[4];
    unsigned char *cur;
} DLIB_DISPLIST;

void dlib_displist_add_data_block__FP13DLIB_DISPLISTP6QWdatai(DLIB_DISPLIST *displist,
                                                              QW *data,
                                                              int count) {
    if (count > 0) {
        do {
            *(QW *)displist->cur = *data;
            ++data;
            displist->cur = (unsigned char *)displist->cur + 0x10;
        } while (--count != 0);
    }
}
