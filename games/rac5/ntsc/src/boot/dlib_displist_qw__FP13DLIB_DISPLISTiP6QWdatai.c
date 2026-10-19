/* dlib_displist_qw__FP13DLIB_DISPLISTiP6QWdatai 0x01E94220..0x01E94280 */
typedef unsigned int QW __attribute__((mode(TI)));

typedef struct {
    char pad0[4];
    unsigned char *cur;
} DLIB_DISPLIST;

typedef struct {
    QW qw;
} QWdata;

extern void dlib_displist_data_unpack_begin__FP13DLIB_DISPLISTii(
    DLIB_DISPLIST *displist, int high, int low);
extern void dlib_displist_add_data_block__FP13DLIB_DISPLISTP6QWdatai(
    DLIB_DISPLIST *displist, QWdata *data, int count);
extern void dlib_displist_data_unpack_end__FP13DLIB_DISPLIST(DLIB_DISPLIST *displist);

void dlib_displist_qw__FP13DLIB_DISPLISTiP6QWdatai(DLIB_DISPLIST *displist, int code,
                                                   QWdata *data, int count) {
    dlib_displist_data_unpack_begin__FP13DLIB_DISPLISTii(displist, 0x6C, code);
    dlib_displist_add_data_block__FP13DLIB_DISPLISTP6QWdatai(displist, data, count);
    dlib_displist_data_unpack_end__FP13DLIB_DISPLIST(displist);
}
