typedef struct {
    char pad0[4];
    unsigned char *cur;
} DLIB_DISPLIST;

typedef struct {
    unsigned long long qw[2];
} QWdata;

extern void dlib_displist_add_call__FP13DLIB_DISPLISTP6QWdata(DLIB_DISPLIST *displist,
                                                              QWdata *data);

void dlib_displist_mpg__FP13DLIB_DISPLISTP6QWdata(DLIB_DISPLIST *displist, QWdata *data) {
    dlib_displist_add_call__FP13DLIB_DISPLISTP6QWdata(displist, data);
}
